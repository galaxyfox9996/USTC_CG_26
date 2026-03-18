#pragma once
#include "shape.h"
#include <vector>
namespace USTC_CG{
    class Polygon : public Shape {
        public:
            Polygon() = default;
            Polygon(std::vector<float> x_list, std::vector<float> y_list)
                : x_list_(std::move(x_list)),
                  y_list_(std::move(y_list)),
                  is_closed_(false)
            {
            }
            virtual ~Polygon() = default;
            void draw(const Config& config) const;
            void update(float x, float y);
            void add_control_point(float x, float y);
            size_t get_point_count() const {
                return x_list_.size();
            }
            void remove_last_control_point(){
                if(!x_list_.empty() && !y_list_.empty())
                {
                    x_list_.pop_back();
                    y_list_.pop_back();
                }
            }
            void set_closed(bool closed){
                is_closed_ = closed;
            }
            bool is_closed() const{
                return is_closed_;
            }
            private:
            std::vector<float> x_list_;
            std::vector<float> y_list_;
            bool is_closed_ = false;
    };
}
