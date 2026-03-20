// HW2_TODO: Implement the IDWWarper class
#pragma once

#include "warper.h"
#include <vector>
#include <utility>
#include <cmath>
#include "imgui.h"
namespace USTC_CG
{
class IDWWarper : public Warper
{
    public:
    IDWWarper() = default;
    virtual ~IDWWarper() = default;
    // HW2_TODO: Implement the warp(...) function with IDW interpolation
    // HW2_TODO: other functions or variables if you need
    //实现IDW函数
    //warp
    std::pair<int,int> warp(int x,int y,int width,int height,std::vector<ImVec2> p,std::vector<ImVec2> q,float power);
    
    private:
    static constexpr double EPSILON = 1e-8;//避免除零错误
};
}  // namespace USTC_CG