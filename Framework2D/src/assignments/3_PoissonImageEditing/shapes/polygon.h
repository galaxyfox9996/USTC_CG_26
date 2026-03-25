#pragma once

#include "shape.h"

#include <vector>

#include <imgui.h>

namespace USTC_CG
{
class Polygon : public Shape
{
   public:
    Polygon() = default;
    ~Polygon() override = default;

    void draw(const Config& config) const override;
    void update(float x, float y) override;
    void add_control_point(float x, float y) override;
    std::vector<std::pair<int, int>> get_interior_pixels() const override;

    void set_closed(bool closed);
    bool is_closed() const;
    std::size_t point_count() const;

   private:
    std::vector<ImVec2> points_;
    ImVec2 preview_point_ = ImVec2(0.f, 0.f);
    bool has_preview_ = false;
    bool is_closed_ = false;
};
}  // namespace USTC_CG
