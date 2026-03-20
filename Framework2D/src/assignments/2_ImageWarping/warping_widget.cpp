#include "warping_widget.h"

#include <cmath>
#include <iostream>

#include "warper/IDW_warper.h"
#include "warper/RBF_warper.h"
#include "imgui.h"
#include "../../../third_party/annoy/src/annoylib.h"
#include "../../../third_party/annoy/src/kissrandom.h"

namespace USTC_CG
{
using uchar = unsigned char;

WarpingWidget::WarpingWidget(
    const std::string& label,
    const std::string& filename)
    : ImageWidget(label, filename)
{
    if (data_)
        back_up_ = std::make_shared<Image>(*data_);
}

void WarpingWidget::draw()
{
    // Draw the image
    ImageWidget::draw();
    // Draw the canvas
    if (flag_enable_selecting_points_)
        select_points();
}

/**
 * @brief WarpingWidget::invert 图像颜色反转函数
 * 该函数将图像中的每个像素的颜色值进行反转，即每个颜色通道的值变为255减去原值
 */
void WarpingWidget::invert()
{
    // 遍历图像的每个像素
    for (int i = 0; i < data_->width(); ++i)
    {                                              // 遍历图像的x坐标（宽度）
        for (int j = 0; j < data_->height(); ++j)  // 遍历图像的y坐标（高度）
        {
            // 获取当前像素的颜色值
            const auto color = data_->get_pixel(i, j);
            // 设置反转后的像素值
            // 每个颜色通道（R、G、B）都进行反转：255 - 原值
            data_->set_pixel(
                i,                                        // x坐标
                j,                                        // y坐标
                { static_cast<uchar>(255 - color[0]),     // 红色通道反转
                  static_cast<uchar>(255 - color[1]),     // 绿色通道反转
                  static_cast<uchar>(255 - color[2]) });  // 蓝色通道反转
        }
    }
    // After change the image, we should reload the image data to the renderer
    update();
}
/**
 * @brief 镜像处理函数，可以对图像进行水平镜像、垂直镜像或同时进行水平和垂直镜像
 * @param is_horizontal 是否进行水平镜像
 * @param is_vertical 是否进行垂直镜像
 */
void WarpingWidget::mirror(bool is_horizontal, bool is_vertical)
{
    // 创建当前图像的临时副本，用于镜像操作
    Image image_tmp(*data_);
    // 获取图像的宽度
    int width = data_->width();
    // 获取图像的高度
    int height = data_->height();

    // 如果需要进行水平镜像
    if (is_horizontal)
    {
        // 如果同时需要进行垂直镜像
        if (is_vertical)
        {
            // 遍历图像的每个像素，进行水平和垂直同时镜像
            for (int i = 0; i < width; ++i)
            {
                for (int j = 0; j < height; ++j)
                {
                    // 将像素从源图像的对称位置复制到目标位置
                    data_->set_pixel(
                        i,
                        j,
                        image_tmp.get_pixel(width - 1 - i, height - 1 - j));
                }
            }
        }
        // 仅进行水平镜像
        else
        {
            // 遍历图像的每个像素，进行水平镜像
            for (int i = 0; i < width; ++i)
            {
                for (int j = 0; j < height; ++j)
                {
                    // 将像素从源图像的水平对称位置复制到目标位置
                    data_->set_pixel(
                        i, j, image_tmp.get_pixel(width - 1 - i, j));
                }
            }
        }
    }
    // 不进行水平镜像，但需要进行垂直镜像
    else
    {
        // 如果需要进行垂直镜像
        if (is_vertical)
        {
            // 遍历图像的每个像素，进行垂直镜像
            for (int i = 0; i < width; ++i)
            {
                for (int j = 0; j < height; ++j)
                {
                    // 将像素从源图像的垂直对称位置复制到目标位置
                    data_->set_pixel(
                        i, j, image_tmp.get_pixel(i, height - 1 - j));
                }
            }
        }
    }

    // After change the image, we should reload the image data to the renderer
    update();
}
// 灰度处理函数，将图像转换为灰度图像
void WarpingWidget::gray_scale()
{
    for (int i = 0; i < data_->width(); ++i)
    {
        for (int j = 0; j < data_->height(); ++j)
        {
            const auto color = data_->get_pixel(i, j);
            uchar gray_value = (color[0] + color[1] + color[2]) / 3;
            data_->set_pixel(i, j, { gray_value, gray_value, gray_value });
        }
    }
    // After change the image, we should reload the image data to the renderer
    update();
}
void WarpingWidget::warping()
{
    // HW2_TODO: You should implement your own warping function that interpolate
    // the selected points.
    // Please design a class for such warping operations, utilizing the
    // encapsulation, inheritance, and polymorphism features of C++.

    // Create a new image to store the result
    Image warped_image(*data_);
    // Initialize the color of result image
    for (int y = 0; y < data_->height(); ++y)
    {
        for (int x = 0; x < data_->width(); ++x)
        {
            warped_image.set_pixel(x, y, { 0, 0, 0 });
        }
    }

    switch (warping_type_)
    {
        case kDefault: break;
        case kFisheye:
        {
            // Example: (simplified) "fish-eye" warping
            // For each (x, y) from the input image, the "fish-eye" warping
            // transfer it to (x', y') in the new image: Note: For this
            // transformation ("fish-eye" warping), one can also calculate the
            // inverse (x', y') -> (x, y) to fill in the "gaps".
            for (int y = 0; y < data_->height(); ++y)
            {
                for (int x = 0; x < data_->width(); ++x)
                {
                    // Apply warping function to (x, y), and we can get (x', y')
                    auto [new_x, new_y] =
                        fisheye_warping(x, y, data_->width(), data_->height());
                    // Copy the color from the original image to the result
                    // image
                    if (new_x >= 0 && new_x < data_->width() && new_y >= 0 &&
                        new_y < data_->height())
                    {
                        std::vector<unsigned char> pixel =
                            data_->get_pixel(x, y);
                        warped_image.set_pixel(new_x, new_y, pixel);
                    }
                }
            }
            break;
        }
        case kIDW:
        {
            // HW2_TODO: Implement the IDW warping
            // use selected points start_points_, end_points_ to construct the
            // map
            //添加控制点
            for(int y = 0;y<data_->height();++y){
                for(int x = 0;x<data_->width();++x){
                    IDWWarper idw;
                    float power = 2;
                    auto [new_x, new_y] = idw.warp(x, y, data_->width(), data_->height(), start_points_, end_points_, power);
                    if(new_x >= 0 && new_x < data_->width() && new_y >= 0 && new_y < data_->height()){
                        std::vector<unsigned char> pixel = data_->get_pixel(x, y);
                        warped_image.set_pixel(new_x, new_y, pixel);
                    }
                }
            }
            break;
        }
        case kRBF:
        {
            // HW2_TODO: Implement the RBF warping
            // use selected points start_points_, end_points_ to construct the
            // map
            for(int x =0;x<data_->height();++x){
                for(int y = 0;y<data_->width();++y){
                    RBFWarper rbf;
                    float power = 1;
                    auto [new_x, new_y] = rbf.warp(x, y, data_->width(), data_->height(), start_points_, end_points_, power);
                    if(new_x >= 0 && new_x < data_->width() && new_y >= 0 && new_y < data_->height()){
                        std::vector<unsigned char> pixel = data_->get_pixel(x, y);
                        warped_image.set_pixel(new_x, new_y, pixel);
                    }
                }
            }
            break;
        }
        case kIDW_plus:
        {
            std::vector<ImVec2> stock;
            for(int y = 0;y<data_->height();++y){
                for(int x = 0;x<data_->width();++x){
                    IDWWarper idw;
                    float power = 2;
                    auto [new_x, new_y] = idw.warp(x, y, data_->width(), data_->height(), start_points_, end_points_, power);
                    ImVec2 vec = {float(new_x), float(new_y)};
                    stock.push_back(vec);
                    if(new_x >= 0 && new_x < data_->width() && new_y >= 0 && new_y < data_->height()){
                        std::vector<unsigned char> pixel = data_->get_pixel(x, y);
                        warped_image.set_pixel(new_x, new_y, pixel);
                    }
                }
            }
            warped_image = fill(stock, warped_image);
            break;
        }
        default: break;
    }

    *data_ = std::move(warped_image);
    update();
}
void WarpingWidget::restore()
{
    *data_ = *back_up_;
    update();
}
void WarpingWidget::set_default()
{
    warping_type_ = kDefault;
}
void WarpingWidget::set_fisheye()
{
    warping_type_ = kFisheye;
}
void WarpingWidget::set_IDW()
{
    warping_type_ = kIDW;
}
void WarpingWidget::set_RBF()
{
    warping_type_ = kRBF;
}
void WarpingWidget::set_IDW_plus()
{
    warping_type_ = kIDW_plus;
}
void WarpingWidget::enable_selecting(bool flag)
{
    flag_enable_selecting_points_ = flag;
}
void WarpingWidget::select_points()
{
    /// Invisible button over the canvas to capture mouse interactions.
    ImGui::SetCursorScreenPos(position_);
    ImGui::InvisibleButton(
        label_.c_str(),
        ImVec2(
            static_cast<float>(image_width_),
            static_cast<float>(image_height_)),
        ImGuiButtonFlags_MouseButtonLeft);
    // Record the current status of the invisible button
    bool is_hovered_ = ImGui::IsItemHovered();
    // Selections
    ImGuiIO& io = ImGui::GetIO();
    if (is_hovered_ && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        draw_status_ = true;
        start_ = end_ =
            ImVec2(io.MousePos.x - position_.x, io.MousePos.y - position_.y);
    }
    if (draw_status_)
    {
        end_ = ImVec2(io.MousePos.x - position_.x, io.MousePos.y - position_.y);
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            start_points_.push_back(start_);
            end_points_.push_back(end_);
            draw_status_ = false;
        }
    }
    // Visualization
    auto draw_list = ImGui::GetWindowDrawList();
    for (size_t i = 0; i < start_points_.size(); ++i)
    {
        ImVec2 s(
            start_points_[i].x + position_.x, start_points_[i].y + position_.y);
        ImVec2 e(
            end_points_[i].x + position_.x, end_points_[i].y + position_.y);
        draw_list->AddLine(s, e, IM_COL32(255, 0, 0, 255), 2.0f);
        draw_list->AddCircleFilled(s, 4.0f, IM_COL32(0, 0, 255, 255));
        draw_list->AddCircleFilled(e, 4.0f, IM_COL32(0, 255, 0, 255));
    }
    if (draw_status_)
    {
        ImVec2 s(start_.x + position_.x, start_.y + position_.y);
        ImVec2 e(end_.x + position_.x, end_.y + position_.y);
        draw_list->AddLine(s, e, IM_COL32(255, 0, 0, 255), 2.0f);
        draw_list->AddCircleFilled(s, 4.0f, IM_COL32(0, 0, 255, 255));
    }
}
void WarpingWidget::init_selections()
{
    start_points_.clear();
    end_points_.clear();
}

std::pair<int, int>
WarpingWidget::fisheye_warping(int x, int y, int width, int height)
{
    float center_x = width / 2.0f;
    float center_y = height / 2.0f;
    float dx = x - center_x;
    float dy = y - center_y;
    float distance = std::sqrt(dx * dx + dy * dy);

    // Simple non-linear transformation r -> r' = f(r)
    float new_distance = std::sqrt(distance) * 10;

    if (distance == 0)
    {
        return { static_cast<int>(center_x), static_cast<int>(center_y) };
    }
    // (x', y')
    float ratio = new_distance / distance;
    int new_x = static_cast<int>(center_x + dx * ratio);
    int new_y = static_cast<int>(center_y + dy * ratio);

    return { new_x, new_y };
}
Image WarpingWidget::fill(std::vector<ImVec2>& stock,Image warped_image){
    size_t n = stock.size(),live = 0;
    float sum,weight;
    float query_vector[2];
    float r,g,b;
    int oldx,oldy;
    unsigned char finalR,finalG,finalB;
    std::vector<unsigned char> blendedPixel;
    std::vector<unsigned char> pixel;
    Annoy::AnnoyIndex<int, float, Annoy::Euclidean, Annoy::Kiss32Random, Annoy::AnnoyIndexSingleThreadedBuildPolicy> index(2);
    std::vector<float> vec;
    std::vector<int> closest_items;
    std::vector<float> distances;
    for(size_t i=0;i<stock.size();++i){
        vec = {float{stock[i].x},float{stock[i].y}};
        index.add_item(i, vec.data());
    }
    index.build(4);
    size_t i = n;
    int y0 = 0,x0=0,wid=data_->width(),heig=data_->height();
    if(warping_type_ == kFisheye){
        x0 = data_->width()/2;
        y0 = data_->height()/2;
        wid =3*x0;
        heig = 3*y0;
    }
    for(int y = 0;y<heig;++y){
        for(int x = 0;x<wid;++x){
            live = 0;
            for(int j = 0;j<n;j++){
                if(stock[j].x==x&&stock[j].y==y){
                    live = 1;
                    break;
                }
            }
            if(live==1){
                continue;
            }
            query_vector[0] = x;
            query_vector[1] = y;
            closest_items.clear();
            distances.clear();
            index.get_nns_by_vector(query_vector,3,-1,&closest_items,&distances);
            sum = 0;weight = 0;
            r = 0.0f,g=0.0f,b=0.0f;
            for(int u= 0;u<3;u++){
                oldy = closest_items[u]/data_->width();
                oldx = closest_items[u]-oldy*data_->width();
                pixel = data_->get_pixel(oldx, oldy);
                weight = 1/distances[u];
                r += pixel[0]*weight;
                g += pixel[1]*weight;
                b += pixel[2]*weight;
            }
            finalR = static_cast<unsigned char>(std::min(255.0f, std::max(0.0f, r/sum)));
            finalG = static_cast<unsigned char>(std::min(255.0f, std::max(0.0f, g/sum)));
            finalB = static_cast<unsigned char>(std::min(255.0f, std::max(0.0f, b/sum)));
            blendedPixel = {finalR,finalG,finalB};
            warped_image.set_pixel(x, y, blendedPixel);
        }
    }
    return warped_image;
}
}  // namespace USTC_CG