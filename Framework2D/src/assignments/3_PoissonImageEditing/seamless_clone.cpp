#include "seamless_clone.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <sstream>
#include <stdexcept>

namespace USTC_CG
{
namespace
{
constexpr std::array<int, 4> kDx = { 1, -1, 0, 0 };
constexpr std::array<int, 4> kDy = { 0, 0, 1, -1 };

template <typename T>
T clamp_value(const T& value, const T& low, const T& high)
{
    return std::min(std::max(value, low), high);
}
}

SeamlessClone::SeamlessClone(
    std::shared_ptr<Image> src_img,
    std::shared_ptr<Image> tar_img,
    std::shared_ptr<Image> src_selected_mask)
    : src_img_(std::move(src_img)),
      tar_img_(std::move(tar_img)),
      src_selected_mask_(std::move(src_selected_mask))
{
}

void SeamlessClone::set_offset(int offset_x, int offset_y)
{
    offset_x_ = offset_x;
    offset_y_ = offset_y;
}

void SeamlessClone::set_gradient_mode(GradientMode mode)
{
    gradient_mode_ = mode;
}

std::shared_ptr<Image> SeamlessClone::solve()
{
    auto result = std::make_shared<Image>(*tar_img_);
    build_domain();
    if (domain_pixels_.empty())
        return result;

    factorize_if_needed();
    if (!factorization_ready_)
        return result;

    const Eigen::MatrixXd rhs = build_rhs();
    const Eigen::MatrixXd solution = solver_.solve(rhs);
    if (solver_.info() != Eigen::Success)
        return result;

    for (int i = 0; i < static_cast<int>(domain_pixels_.size()); ++i)
    {
        const auto& pixel = domain_pixels_[i];
        std::vector<unsigned char> color = result->get_pixel(pixel.tar_x, pixel.tar_y);
        const int used_channels = std::min(3, result->channels());
        for (int channel = 0; channel < used_channels; ++channel)
        {
            color[channel] =
                static_cast<unsigned char>(std::round(clamp_color(solution(i, channel))));
        }
        result->set_pixel(pixel.tar_x, pixel.tar_y, color);
    }

    return result;
}

double SeamlessClone::clamp_color(double value)
{
    return clamp_value(value, 0.0, 255.0);
}

std::vector<unsigned char> SeamlessClone::get_pixel_clamped(
    const std::shared_ptr<Image>& image,
    int x,
    int y) const
{
    const int clamped_x = clamp_value(x, 0, image->width() - 1);
    const int clamped_y = clamp_value(y, 0, image->height() - 1);
    return image->get_pixel(clamped_x, clamped_y);
}

bool SeamlessClone::is_in_target(int x, int y) const
{
    return 0 <= x && x < tar_img_->width() && 0 <= y && y < tar_img_->height();
}

void SeamlessClone::build_domain()
{
    domain_pixels_.clear();
    src_to_domain_index_.clear();

    std::ostringstream signature_builder;
    for (int y = 0; y < src_selected_mask_->height(); ++y)
    {
        for (int x = 0; x < src_selected_mask_->width(); ++x)
        {
            if (src_selected_mask_->get_pixel(x, y)[0] == 0)
                continue;

            const int tar_x = x + offset_x_;
            const int tar_y = y + offset_y_;
            if (!is_in_target(tar_x, tar_y))
                continue;

            const int key =
                y * src_selected_mask_->width() + x;
            src_to_domain_index_[key] = static_cast<int>(domain_pixels_.size());
            domain_pixels_.push_back(PixelInfo { x, y, tar_x, tar_y });
            signature_builder << key << ';';
        }
    }

    const std::string new_signature = signature_builder.str();
    if (new_signature != domain_signature_)
    {
        domain_signature_ = new_signature;
        factorization_ready_ = false;
    }
}

void SeamlessClone::factorize_if_needed()
{
    if (factorization_ready_)
        return;

    const int n = static_cast<int>(domain_pixels_.size());
    matrix_a_.resize(n, n);
    std::vector<Eigen::Triplet<double>> triplets;
    triplets.reserve(static_cast<std::size_t>(n) * 5);

    for (int i = 0; i < n; ++i)
    {
        const auto& pixel = domain_pixels_[i];
        double diagonal = 0.0;
        for (int dir = 0; dir < 4; ++dir)
        {
            const int neighbor_src_x = pixel.src_x + kDx[dir];
            const int neighbor_src_y = pixel.src_y + kDy[dir];
            const int neighbor_tar_x = pixel.tar_x + kDx[dir];
            const int neighbor_tar_y = pixel.tar_y + kDy[dir];

            if (!is_in_target(neighbor_tar_x, neighbor_tar_y))
                continue;

            diagonal += 1.0;
            const int neighbor_key =
                neighbor_src_y * src_selected_mask_->width() + neighbor_src_x;
            const auto it = src_to_domain_index_.find(neighbor_key);
            if (it != src_to_domain_index_.end())
            {
                triplets.emplace_back(i, it->second, -1.0);
            }
        }

        triplets.emplace_back(i, i, diagonal);
    }

    matrix_a_.setFromTriplets(triplets.begin(), triplets.end());
    solver_.compute(matrix_a_);
    factorization_ready_ = (solver_.info() == Eigen::Success);
}

Eigen::MatrixXd SeamlessClone::build_rhs() const
{
    const int n = static_cast<int>(domain_pixels_.size());
    Eigen::MatrixXd rhs = Eigen::MatrixXd::Zero(n, 3);

    for (int i = 0; i < n; ++i)
    {
        const auto& pixel = domain_pixels_[i];
        for (int dir = 0; dir < 4; ++dir)
        {
            const int neighbor_src_x = pixel.src_x + kDx[dir];
            const int neighbor_src_y = pixel.src_y + kDy[dir];
            const int neighbor_tar_x = pixel.tar_x + kDx[dir];
            const int neighbor_tar_y = pixel.tar_y + kDy[dir];

            if (!is_in_target(neighbor_tar_x, neighbor_tar_y))
                continue;

            const int neighbor_key =
                neighbor_src_y * src_selected_mask_->width() + neighbor_src_x;
            const bool neighbor_in_domain =
                src_to_domain_index_.find(neighbor_key) != src_to_domain_index_.end();

            for (int channel = 0; channel < 3; ++channel)
            {
                rhs(i, channel) += guidance(
                    pixel.src_x,
                    pixel.src_y,
                    pixel.tar_x,
                    pixel.tar_y,
                    neighbor_src_x,
                    neighbor_src_y,
                    neighbor_tar_x,
                    neighbor_tar_y,
                    channel);

                if (!neighbor_in_domain)
                {
                    rhs(i, channel) += static_cast<double>(
                        tar_img_->get_pixel(neighbor_tar_x, neighbor_tar_y)[channel]);
                }
            }
        }
    }

    return rhs;
}

double SeamlessClone::guidance(
    int src_x,
    int src_y,
    int tar_x,
    int tar_y,
    int neighbor_src_x,
    int neighbor_src_y,
    int neighbor_tar_x,
    int neighbor_tar_y,
    int channel) const
{
    const auto src_pixel = get_pixel_clamped(src_img_, src_x, src_y);
    const auto src_neighbor = get_pixel_clamped(src_img_, neighbor_src_x, neighbor_src_y);
    const double src_grad =
        static_cast<double>(src_pixel[channel]) -
        static_cast<double>(src_neighbor[channel]);

    if (gradient_mode_ == GradientMode::kImport)
        return src_grad;

    const auto tar_pixel = get_pixel_clamped(tar_img_, tar_x, tar_y);
    const auto tar_neighbor =
        get_pixel_clamped(tar_img_, neighbor_tar_x, neighbor_tar_y);
    const double tar_grad =
        static_cast<double>(tar_pixel[channel]) -
        static_cast<double>(tar_neighbor[channel]);

    return std::abs(src_grad) >= std::abs(tar_grad) ? src_grad : tar_grad;
}
}  // namespace USTC_CG
