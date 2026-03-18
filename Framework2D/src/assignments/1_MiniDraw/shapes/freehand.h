#pragma once
#include "shape.h"
#include <vector>
namespace USTC_CG{
    class Freehand :public Shape{
        public:
            Freehand() = default;
            Freehand(std::vector<float> x_list,std::vector<float> y_list)
                : x_list_(std::move(x_list)),
                  y_list_(std::move(y_list))
            {
            }
            virtual ~Freehand() = default;

            void draw(const Config& config) const;
            void update(float x, float y);
            void add_control_point(float x,float y);

            //辅助函数
            size_t get_point_count() const{
                return x_list_.size();
            }
            void clear(){
                x_list_.clear();
                y_list_.clear();
            }

        private:
            std::vector<float> x_list_;
            std::vector<float> y_list_;
    };
}
