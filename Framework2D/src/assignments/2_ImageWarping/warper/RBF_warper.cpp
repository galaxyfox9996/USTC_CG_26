#include "RBF_warper.h"
#include <iostream>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include "imgui.h"
#include <Eigen/Dense>

namespace USTC_CG{
    double RBFWarper::distance(const ImVec2 p1,const ImVec2 p2){
        return sqrt((p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y));
    }


    std::pair<int,int> RBFWarper::warp(int x,int y,int weight,int height,std::vector<ImVec2> p,std::vector<ImVec2> q,float power){
        size_t n = p.size();
        if(n==1){
            return {x+q[0].x-p[0].x,y+q[0].y-p[0].y};
        }
        //计算径向基函数半径
        std::vector<double> radii;
        for(size_t i =0;i<n;i++){
            radii.push_back(0.0f);
            for(size_t j = 0;j<n;j++){
                if(i==j){
                    continue;
                }
                double d = distance(p[i],p[j]);
                if(d<radii[i]||radii[i]==0){
                    radii[i] = d;
                }
            }
        }
        //计算权重矩阵
        Eigen::MatrixXd G(n,n);
        for(size_t i = 0;i<n;i++){
            for(size_t j = 0;j<n;j++){
                double d = distance(p[i],p[j]);
                G(i,j) = pow(d*d+radii[j]*radii[j],power/2.0);
            }
        }
        Eigen::VectorXd Rx(n);
        Eigen::VectorXd Ry(n);
        for(size_t i = 0;i<n;i++){
            Rx(i) = q[i].x-p[i].x;
            Ry(i) = q[i].y-p[i].y;
        }

        //求解
        Eigen::VectorXd X = G.fullPivLu().solve(Rx);
        Eigen::VectorXd Y = G.fullPivLu().solve(Ry);

        //插值
        double x_new = x;
        double y_new = y;
        float x_old = x;
        float y_old = y;
        for(size_t i=0;i<p.size();++i){
            double d = distance({x_old,y_old},p[i]);
            double g =std::pow(d*d+radii[i]*radii[i],power/2.0);
            x_new += X(i)*g;
            y_new += Y(i)*g;
        }

        return {static_cast<int>(x_new),static_cast<int>(y_new)};
    }
}