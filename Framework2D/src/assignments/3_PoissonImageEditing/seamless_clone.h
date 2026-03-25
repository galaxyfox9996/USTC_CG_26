#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <Eigen/Sparse>

#include "common/image.h"

namespace USTC_CG
{
class SeamlessClone
{
   public:
    enum class GradientMode
    {
        kImport = 0,
        kMixed = 1
    };

    SeamlessClone(
        std::shared_ptr<Image> src_img,
        std::shared_ptr<Image> tar_img,
        std::shared_ptr<Image> src_selected_mask);

    void set_offset(int offset_x, int offset_y);
    void set_gradient_mode(GradientMode mode);
    std::shared_ptr<Image> solve();

   private:
    struct PixelInfo
    {
        int src_x = 0;
        int src_y = 0;
        int tar_x = 0;
        int tar_y = 0;
    };

    static double clamp_color(double value);
    std::vector<unsigned char> get_pixel_clamped(
        const std::shared_ptr<Image>& image,
        int x,
        int y) const;
    bool is_in_target(int x, int y) const;
    void build_domain();
    void factorize_if_needed();
    Eigen::MatrixXd build_rhs() const;
    double guidance(
        int src_x,
        int src_y,
        int tar_x,
        int tar_y,
        int neighbor_src_x,
        int neighbor_src_y,
        int neighbor_tar_x,
        int neighbor_tar_y,
        int channel) const;

    std::shared_ptr<Image> src_img_;
    std::shared_ptr<Image> tar_img_;
    std::shared_ptr<Image> src_selected_mask_;
    int offset_x_ = 0;
    int offset_y_ = 0;
    GradientMode gradient_mode_ = GradientMode::kImport;

    std::vector<PixelInfo> domain_pixels_;
    std::unordered_map<int, int> src_to_domain_index_;
    std::string domain_signature_;

    Eigen::SparseMatrix<double> matrix_a_;
    Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver_;
    bool factorization_ready_ = false;
};
}  // namespace USTC_CG
