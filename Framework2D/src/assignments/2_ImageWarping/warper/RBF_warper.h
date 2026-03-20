// HW2_TODO: Implement the RBFWarper class
#pragma once

#include "warper.h"
#include <vector>
#include "imgui.h"

namespace USTC_CG
{
class RBFWarper : public Warper
{
   public:
    RBFWarper() = default;
    virtual ~RBFWarper() = default;
    // HW2_TODO: Implement the warp(...) function with RBF interpolation
    std::pair<int,int> warp(int x,int y,int width,int height,std::vector<ImVec2> p,std::vector<ImVec2> q,float power);
    // HW2_TODO: other functions or variables if you need
    private :
    double distance(const ImVec2 p1,const ImVec2 p2);
};
}  // namespace USTC_CG