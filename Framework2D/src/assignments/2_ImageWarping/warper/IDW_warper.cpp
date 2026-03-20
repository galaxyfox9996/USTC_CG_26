#include "IDW_warper.h"

#include <iostream>
#include <cmath>
#include <algorithm>
#include "imgui.h"
namespace USTC_CG
{
    // HW2_TODO: Implement the warp(...) function with IDW interpolation
std::pair<int,int> IDWWarper::warp(int x,int y,int width,int height,std::vector<ImVec2> p,std::vector<ImVec2> q,float power)
{
    double sum_weights = 0.0f;
    double new_x = 0.0f, new_y = 0.0f;
    std::vector<double> sigma;

    for (size_t i = 0; i < p.size(); ++i) {
        double dx = x - p[i].x;
        double dy = y - p[i].y;
        double distance = std::sqrt(dx * dx + dy * dy);

        // 避免除零错误，若点恰好在控制点上，则直接返回目标点
        if (distance < 1e-6) {
            return {static_cast<int>(q[i].x), static_cast<int>(q[i].y)};
        }
        sigma.push_back(1.0f / std::pow(distance, power));
        sum_weights +=sigma[i];
    

    double A[2][2] = {0}, B[2][2] = {0}, T[2][2] = {0};
    ImVec2 pi = p[i], qi = q[i];

    for (int j = 0; j < p.size(); ++j) {
        if (i == j) continue;

        double dx = p[j].x - pi.x;
        double dy = p[j].y - pi.y;

        double sigma_j = 1.0f / std::pow(std::sqrt(dx * dx + dy * dy), power);

        A[0][0] += sigma_j * dx * dx;
        A[0][1] += sigma_j * dx * dy;
        A[1][0] += sigma_j * dx * dy;
        A[1][1] += sigma_j * dy * dy;

        double qdx = q[j].x - qi.x;
        double qdy = q[j].y - qi.y;

        B[0][0] += sigma_j * qdx * dx;
        B[0][1] += sigma_j * qdx * dy;
        B[1][0] += sigma_j * qdy * dx;
        B[1][1] += sigma_j * qdy * dy;
    }

    // 计算 T = A^-1 * B
    double det = A[0][0] * A[1][1] - A[0][1] * A[1][0];
    double invA[2][2] = {
        {A[1][1] / det, -A[0][1] / det},
        {-A[1][0] / det, A[0][0] / det}
    };

    T[0][0] = invA[0][0] * B[0][0] + invA[1][0] * B[0][1];
    T[0][1] = invA[0][1] * B[0][0] + invA[1][1] * B[0][1];
    T[1][0] = invA[0][0] * B[1][0] + invA[1][0] * B[1][1];
    T[1][1] = invA[0][1] * B[1][0] + invA[1][1] * B[1][1];
    new_x+=(q[i].x+T[0][0]*(x-p[i].x)+T[0][1]*(y-p[i].y))*sigma[i];
    new_y+=(q[i].y+T[1][0]*(x-p[i].x)+T[1][1]*(y-p[i].y))*sigma[i];
    }

    // 确保坐标不超出图片范围
    int final_x = new_x/sum_weights;
    int final_y = new_y/sum_weights;

    return {final_x, final_y};
}

    // HW2_TODO: other functions or variables if you need

}  // namespace USTC_CG