#include "freehand.h"
#include <imgui.h>
#include <math.h>
namespace USTC_CG{
    void Freehand :: draw(const Config& config)const{
        if (x_list_.size() < 2 || y_list_.size() < 2 || x_list_.size() != y_list_.size()) {
            return;  // 点数不够，不绘制
        }
        
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        const ImU32 color = IM_COL32(
            config.line_color[0],
            config.line_color[1],
            config.line_color[2],
            config.line_color[3]);
        
        for(size_t i = 0;i<x_list_.size()-1;i++){
            ImVec2 p1(x_list_[i]+config.bias[0],y_list_[i]+config.bias[1]);
            ImVec2 p2(x_list_[i+1]+config.bias[0],y_list_[i+1]+config.bias[1]);
            draw_list->AddLine(p1,p2,color,config.line_thickness);
        }
    }

    void Freehand::update(float x,float y){
        add_control_point(x,y);
    }

    void Freehand::add_control_point(float x,float y){
        x_list_.push_back(x);
        y_list_.push_back(y);
    }
}