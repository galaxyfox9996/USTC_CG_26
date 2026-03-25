#include "freehand.h"

#include <algorithm>
#include <cmath>

#include <imgui.h>

namespace USTC_CG
{
void Freehand::draw(const Config& config) const
{
    if (points_.empty())
        return;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const ImU32 color = IM_COL32(
        config.line_color[0],
        config.line_color[1],
        config.line_color[2],
        config.line_color[3]);

    for (std::size_t i = 1; i < points_.size(); ++i)
    {
        draw_list->AddLine(
            ImVec2(
                config.bias[0] + points_[i - 1].x,
                config.bias[1] + points_[i - 1].y),
            ImVec2(config.bias[0] + points_[i].x, config.bias[1] + points_[i].y),
            color,
            config.line_thickness);
    }

    if (is_closed_ && points_.size() >= 3)
    {
        draw_list->AddLine(
            ImVec2(
                config.bias[0] + points_.back().x,
                config.bias[1] + points_.back().y),
            ImVec2(config.bias[0] + points_.front().x, config.bias[1] + points_.front().y),
            color,
            config.line_thickness);
    }
}

void Freehand::update(float x, float y)
{
    add_control_point(x, y);
}

void Freehand::add_control_point(float x, float y)
{
    const ImVec2 point(x, y);
    if (!points_.empty())
    {
        const ImVec2& last = points_.back();
        if (std::abs(last.x - point.x) < 0.5f && std::abs(last.y - point.y) < 0.5f)
            return;
    }
    points_.push_back(point);
}

std::vector<std::pair<int, int>> Freehand::get_interior_pixels() const
{
    std::vector<std::pair<int, int>> interior_pixels;
    if (!is_closed_ || points_.size() < 3)
        return interior_pixels;

    float min_y = points_.front().y;
    float max_y = points_.front().y;
    for (const auto& point : points_)
    {
        min_y = std::min(min_y, point.y);
        max_y = std::max(max_y, point.y);
    }

    const int y_begin = static_cast<int>(std::floor(min_y));
    const int y_end = static_cast<int>(std::ceil(max_y));

    for (int y = y_begin; y <= y_end; ++y)
    {
        const double scan_y = static_cast<double>(y) + 0.5;
        std::vector<double> intersections;

        for (std::size_t i = 0; i < points_.size(); ++i)
        {
            const ImVec2& a = points_[i];
            const ImVec2& b = points_[(i + 1) % points_.size()];
            const double ay = static_cast<double>(a.y);
            const double by = static_cast<double>(b.y);
            const bool crosses =
                (ay <= scan_y && scan_y < by) || (by <= scan_y && scan_y < ay);
            if (!crosses)
                continue;

            const double t = (scan_y - ay) / (by - ay);
            const double x = static_cast<double>(a.x) +
                             t * (static_cast<double>(b.x) - static_cast<double>(a.x));
            intersections.push_back(x);
        }

        std::sort(intersections.begin(), intersections.end());
        for (std::size_t i = 0; i + 1 < intersections.size(); i += 2)
        {
            const int x_begin = static_cast<int>(std::ceil(intersections[i] - 0.5));
            const int x_end =
                static_cast<int>(std::floor(intersections[i + 1] - 0.5));
            for (int x = x_begin; x <= x_end; ++x)
            {
                interior_pixels.emplace_back(x, y);
            }
        }
    }

    return interior_pixels;
}

void Freehand::set_closed(bool closed)
{
    is_closed_ = closed;
}

bool Freehand::is_closed() const
{
    return is_closed_;
}

std::size_t Freehand::point_count() const
{
    return points_.size();
}
}  // namespace USTC_CG
