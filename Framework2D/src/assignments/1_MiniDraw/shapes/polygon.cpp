#include "polygon.h"
#include <imgui.h>
#include <math.h>
namespace USTC_CG{
    void Polygon::draw(const Config& config) const{
        if(x_list_.size() < 2 || y_list_.size() < 2 || x_list_.size() != y_list_.size())
        {
            return;
        }
        ImDrawList* drawlist = ImGui::GetWindowDrawList();
        const ImU32 color = IM_COL32(
            config.line_color[0],
            config.line_color[1],
            config.line_color[2],
            config.line_color[3]);
        for(size_t i = 0;i<x_list_.size()-1;i++){
            ImVec2 p1(x_list_[i]+config.bias[0],y_list_[i]+config.bias[1]);
            ImVec2 p2(x_list_[i+1]+config.bias[0],y_list_[i+1]+config.bias[1]);
            drawlist->AddLine(p1,p2,color,config.line_thickness);
        }
        // Connect the last point to the first point to close the polygon
        ImVec2 p_last(x_list_.back()+config.bias[0],y_list_.back()+config.bias[1]);
        ImVec2 p_first(x_list_.front()+config.bias[0],y_list_.front()+config.bias[1]);
        //**只有在封闭状态且点数>=3时，才连接首尾点**
        if(is_closed_ && x_list_.size()>=3){
            drawlist->AddLine(p_last,p_first,color,config.line_thickness);
        }
    }
    void Polygon::add_control_point(float x, float y){
        x_list_.push_back(x);
        y_list_.push_back(y);
    }
    void Polygon::update(float x, float y){
        if(x_list_.empty() || y_list_.empty())
        {
            return;
        }
        x_list_.back() = x;
        y_list_.back() = y;
    }
}