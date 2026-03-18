#include "canvas_widget.h"

#include <cmath>
#include <iostream>

#include "imgui.h"
#include "shapes/line.h"
#include "shapes/rect.h"
#include "shapes/ellipse.h"
#include "shapes/polygon.h"
#include "shapes/freehand.h"

namespace USTC_CG
{
void Canvas::draw()
{
    draw_background();
    // HW1_TODO: more interaction events
    if (is_hovered_ && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        mouse_click_event();
    if(is_hovered_ && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        mouse_right_click_event();
    mouse_move_event();
    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
        mouse_release_event();
    

    draw_shapes();
}

void Canvas::set_attributes(const ImVec2& min, const ImVec2& size)
{
    canvas_min_ = min;
    canvas_size_ = size;
    canvas_minimal_size_ = size;
    canvas_max_ =
        ImVec2(canvas_min_.x + canvas_size_.x, canvas_min_.y + canvas_size_.y);
}

void Canvas::show_background(bool flag)
{
    show_background_ = flag;
}

void Canvas::set_default()
{
    draw_status_ = false;
    shape_type_ = kDefault;
}

void Canvas::set_line()
{
    draw_status_ = false;
    shape_type_ = kLine;
}

void Canvas::set_rect()
{
    draw_status_ = false;
    shape_type_ = kRect;
}

// HW1_TODO: more shape types, implements
void Canvas::set_ellipse()
{
    draw_status_ = false;
    shape_type_ = kEllipse;
}

void Canvas::set_polygon()
{
    draw_status_ = false;
    shape_type_ = kPolygon;
}

void Canvas::set_freehand()
{
    draw_status_ = false;
    shape_type_ = kFreehand;
}

void Canvas::clear_shape_list()
{
    shape_list_.clear();
}

void Canvas::draw_background()
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    if (show_background_)
    {
        // Draw background recrangle
        draw_list->AddRectFilled(canvas_min_, canvas_max_, background_color_);
        // Draw background border
        draw_list->AddRect(canvas_min_, canvas_max_, border_color_);
    }
    /// Invisible button over the canvas to capture mouse interactions.
    ImGui::SetCursorScreenPos(canvas_min_);
    ImGui::InvisibleButton(
        label_.c_str(), canvas_size_, ImGuiButtonFlags_MouseButtonLeft);
    // Record the current status of the invisible button
    is_hovered_ = ImGui::IsItemHovered();
    is_active_ = ImGui::IsItemActive();
}

void Canvas::draw_shapes()
{
    Shape::Config s = { .bias = { canvas_min_.x, canvas_min_.y } };
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // ClipRect can hide the drawing content outside of the rectangular area
    draw_list->PushClipRect(canvas_min_, canvas_max_, true);
    for (const auto& shape : shape_list_)
    {
        shape->draw(s);
    }
    if (draw_status_ && current_shape_)
    {
        current_shape_->draw(s);
    }
    draw_list->PopClipRect();
}

void Canvas::mouse_click_event()
{
    // HW1_TODO: Drawing rule for more primitives
    if (!draw_status_)
    {
        draw_status_ = true;
        start_point_ = end_point_ = mouse_pos_in_canvas();
        switch (shape_type_)
        {
            case USTC_CG::Canvas::kDefault:
            {
                break;
            }
            case USTC_CG::Canvas::kLine:
            {
                current_shape_ = std::make_shared<Line>(
                    start_point_.x, start_point_.y, end_point_.x, end_point_.y);
                break;
            }
            case USTC_CG::Canvas::kRect:
            {
                current_shape_ = std::make_shared<Rect>(
                    start_point_.x, start_point_.y, end_point_.x, end_point_.y);
                break;
            }
            // HW1_TODO: case USTC_CG::Canvas::kEllipse:
            case USTC_CG::Canvas::kEllipse:
            {
                current_shape_ = std::make_shared<Ellipse>(
                    start_point_.x, start_point_.y, end_point_.x, end_point_.y);
                break;
            }
            case USTC_CG::Canvas::kPolygon:
            {
                auto polygon = std::make_shared<Polygon>();
                polygon->add_control_point(start_point_.x, start_point_.y);
                // polygon->add_control_point(end_point_.x, end_point_.y);
                current_shape_ = polygon;
                is_drawing_polygon_ = true;
                break;
            }
            case USTC_CG::Canvas::kFreehand:
            {
                auto freehand = std::make_shared<Freehand>();
                freehand->add_control_point(start_point_.x, start_point_.y);
                current_shape_ = freehand;
                last_point_ = start_point_;
                break;
            }
            default: break;

        }
    }
    else if(is_drawing_polygon_ && shape_type_ == USTC_CG::Canvas::kPolygon){
        auto polygon = std::dynamic_pointer_cast<Polygon>(current_shape_);
        if(polygon){
            ImVec2 click_pos = mouse_pos_in_canvas();
            if(polygon->get_point_count()>=2){
                float dist_to_start = sqrt(pow(click_pos.x - start_point_.x, 2) + pow(click_pos.y - start_point_.y, 2));
                if(dist_to_start < 10.0f && polygon->get_point_count()>=3){
                    polygon->remove_last_control_point();
                    polygon->set_closed(true);
                    shape_list_.push_back(current_shape_);
                    draw_status_ = false;
                    current_shape_.reset();
                    is_drawing_polygon_ = false;
                    return;
                }
            }
            polygon->add_control_point(click_pos.x, click_pos.y);
            end_point_ = click_pos;
            polygon->update(end_point_.x, end_point_.y);
        }
    }
    else if(shape_type_ == USTC_CG::Canvas::kFreehand){
        if(ImGui::IsMouseDoubleClicked(0)){
            draw_status_ = false;
            if(current_shape_){
                auto freehand = std::dynamic_pointer_cast<Freehand>(current_shape_);
                if(freehand&& freehand->get_point_count()>=2){
                    shape_list_.push_back(current_shape_);
                }
                current_shape_.reset();
            }
        }
    }
    else
    {
        draw_status_ = false;
        if (current_shape_)
        {
            shape_list_.push_back(current_shape_);
            current_shape_.reset();
        }
    }
}

void Canvas::mouse_move_event()
{
    // HW1_TODO: Drawing rule for more primitives
    if (draw_status_)
    {
        end_point_ = mouse_pos_in_canvas();
        if (current_shape_)
        {
            if(shape_type_ == USTC_CG::Canvas::kPolygon){
                auto polygon = std::dynamic_pointer_cast<Polygon>(current_shape_);
                if(polygon){
                    if(polygon->get_point_count()==1){
                        polygon->add_control_point(end_point_.x, end_point_.y);
                    }
                    else{
                        polygon->update(end_point_.x, end_point_.y);
                    }
                }
            }
            else if(shape_type_ == USTC_CG::Canvas::kFreehand){
                auto freehand = std::dynamic_pointer_cast<Freehand>(current_shape_);
                if(freehand){
                    float dist = sqrt(pow(end_point_.x - last_point_.x, 2) + pow(end_point_.y - last_point_.y, 2));
                    if(dist > freehand_min_distance_){
                        freehand->add_control_point(end_point_.x, end_point_.y);
                        last_point_ = end_point_;
                    }
                }
            }
            else{
                current_shape_->update(end_point_.x, end_point_.y);
            }
        }
    }
}

void Canvas::mouse_release_event()
{
    // HW1_TODO: Drawing rule for more primitives
    // 对于多边形，鼠标释放不需要特殊处理
    if (shape_type_ == USTC_CG::Canvas::kPolygon && is_drawing_polygon_)
    {
        return;
    }
    if(draw_status_&&shape_type_ != USTC_CG::Canvas::kPolygon)
    {
        draw_status_ = false;
        if (current_shape_)
        {
            shape_list_.push_back(current_shape_);
            current_shape_.reset();
        }
    }
    else if(draw_status_ && shape_type_ == USTC_CG::Canvas::kFreehand){
        auto freehand = std::dynamic_pointer_cast<Freehand>(current_shape_);
        if(freehand && freehand->get_point_count()>=2){
            shape_list_.push_back(current_shape_);
        }
        current_shape_.reset();
        draw_status_ = false;
    }
}

ImVec2 Canvas::mouse_pos_in_canvas() const
{
    ImGuiIO& io = ImGui::GetIO();
    const ImVec2 mouse_pos_in_canvas(
        io.MousePos.x - canvas_min_.x, io.MousePos.y - canvas_min_.y);
    return mouse_pos_in_canvas;
}
void Canvas::mouse_right_click_event(){
    if(draw_status_ && shape_type_ == USTC_CG::Canvas::kPolygon){
        auto polygon = std::dynamic_pointer_cast<Polygon>(current_shape_);
        if(polygon){
            polygon->remove_last_control_point();
            polygon->set_closed(true);
            shape_list_.push_back(current_shape_);
            draw_status_ = false;
            current_shape_.reset();
            is_drawing_polygon_ = false;
        }
    }
    else if(draw_status_ &&shape_type_ == USTC_CG::Canvas::kFreehand){
        draw_status_ = false;
        current_shape_.reset();
    }
}
}  // namespace USTC_CG