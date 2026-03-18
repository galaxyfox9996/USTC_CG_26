#include "ellipse.h"
#include <imgui.h>
#include <math.h>
namespace USTC_CG
{
    void Ellipse::draw(const Config& config)const
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        const float center_x = (start_point_x_ + end_point_x_) / 2.0f + config.bias[0];
        const float center_y = (start_point_y_ + end_point_y_) / 2.0f + config.bias[1];
        ImVec2 center(center_x, center_y);
        const float radius_x = fabs(end_point_x_ - start_point_x_) / 2.0f;
        const float radius_y = fabs(end_point_y_ - start_point_y_) / 2.0f;
        ImVec2 radius(radius_x, radius_y);
        const ImU32 color = IM_COL32(
            config.line_color[0],
            config.line_color[1],
            config.line_color[2],
            config.line_color[3]);
        draw_list->AddEllipse(
            center,
            radius,
            color,
            0.f,  // No rotation
            0,  // No specific flags
            config.line_thickness
        );    
    }
    void Ellipse::update(float x, float y)
    {
        end_point_x_ = x;
        end_point_y_ = y;
    }
}