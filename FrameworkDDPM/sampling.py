import torch
from forward_noising import (
    get_index_from_list,
    sqrt_one_minus_alphas_cumprod,
    betas,
    posterior_variance,
    sqrt_recip_alphas,
    forward_diffusion_sample,
    T,
)
import matplotlib.pyplot as plt
from dataloader import show_tensor_image
from unet import SimpleUnet
import numpy as np
import cv2 as cv
from torchvision.utils import save_image
import os


# TODO: 你需要在这个函数中实现单步去噪过程
@torch.no_grad()
def sample_timestep(model, x, t, device):
    """
    执行单步去噪过程。
    
    参数：
        model: 训练好的 UNet 模型
        x: 当前时间步的噪声图像，形状 (batch_size, 3, img_size, img_size)
        t: 当前时间步，形状 (batch_size,)
        device: 计算设备 ('cpu' 或 'cuda')
    
    返回：
        去噪后的图像
    """
    # 获取模型预测的噪声
    betas_t = get_index_from_list(betas, t, x.shape)
    sqrt_one_minus_alphas_cumprod_t = get_index_from_list(sqrt_one_minus_alphas_cumprod, t, x.shape)
    sqrt_recip_alphas_t = get_index_from_list(sqrt_recip_alphas, t, x.shape)
    
    # 模型预测噪声
    model_mean = sqrt_recip_alphas_t * (x - betas_t * model(x, t) / sqrt_one_minus_alphas_cumprod_t)
    
    # 如果不是最后一步，添加方差
    if t[0] == 0:
        return model_mean
    else:
        posterior_variance_t = get_index_from_list(posterior_variance, t, x.shape)
        noise = torch.randn_like(x)
        return model_mean + torch.sqrt(posterior_variance_t) * noise

# TODO: 你需要在这个函数中完成对纯高斯噪声的去噪，并输出对应的去噪图片
# 你需要调用上面的sample_timestep函数，以实现单步去噪
@torch.no_grad()
def sample_plot_image(model, device, img_size, T):
    """
    从纯高斯噪声生成图像。
    
    参数：
        model: 训练好的 UNet 模型
        device: 计算设备
        img_size: 生成图像的大小
        T: 总时间步数
    
    返回：
        生成的图像张量，形状 (1, 3, img_size, img_size)
    """
    # 从纯高斯噪声开始
    img = torch.randn((1, 3, img_size, img_size), device=device)
    
    # 从 T-1 逆向去噪到 0
    for i in range(T - 1, -1, -1):
        t = torch.full((1,), i, dtype=torch.long, device=device)
        img = sample_timestep(model, img, t, device)
    
    return img

# TODO: 你需要在这个函数中完成模型以及其他相关资源的加载，并调用sample_plot_image进行去噪，以生成图片
def test_image_generation():
    """
    测试图像生成功能。加载训练好的模型，生成图像并保存。
    """
    # 配置参数
    device = "cuda" if torch.cuda.is_available() else "cpu"
    img_size = 64
    T = 300
    model_path = "./ddpm_mse_epochs_5000.pth"
    output_dir = "./generated_images"
    
    # 创建输出目录
    os.makedirs(output_dir, exist_ok=True)
    
    # 加载模型
    model = SimpleUnet()
    
    # 检查模型文件是否存在
    if not os.path.exists(model_path):
        print(f"模型文件 {model_path} 不存在，请先训练模型")
        return
    
    model.load_state_dict(torch.load(model_path, map_location=device))
    model.to(device)
    model.eval()
    
    print(f"使用设备: {device}")
    print("开始生成图像...")
    
    # 生成多张图像
    num_images = 4
    for i in range(num_images):
        print(f"生成第 {i+1}/{num_images} 张图像...")
        generated_img = sample_plot_image(model, device, img_size, T)
        
        # 保存图像
        img_path = os.path.join(output_dir, f"generated_{i}.png")
        save_image(generated_img, img_path)
        print(f"图像已保存到: {img_path}")
    
    print("图像生成完成！")

# TODO：你需要在这个函数中实现图像的补充
# Follows: RePaint: Inpainting using Denoising Diffusion Probabilistic Models
@torch.no_grad()
def inpaint(model, device, img, mask, t_max=50):
    """
    图像补全函数，基于 RePaint 算法。
    
    参数：
        model: 训练好的 UNet 模型
        device: 计算设备
        img: 原始图像，形状 (1, 3, img_size, img_size)，值范围 [-1, 1]
        mask: 掩码，形状 (1, 1, img_size, img_size)，1 表示需要补全的区域，0 表示保留的区域
        t_max: 最大时间步数
    
    返回：
        补全后的图像
    """
    # 获取图像大小
    img_size = img.shape[-1]
    
    # 从 t_max 逆向去噪到 0
    for i in range(t_max - 1, -1, -1):
        t = torch.full((1,), i, dtype=torch.long, device=device)
        
        # 单步去噪
        img = sample_timestep(model, img, t, device)
        
        # 使用掩码恢复原始图像的已知部分
        # 对原始图像进行加噪
        original_noisy, _ = forward_diffusion_sample(img, t, device)
        
        # 根据掩码混合：保留已知部分，更新未知部分
        img = img * mask + original_noisy * (1 - mask)
    
    return img

# TODO: 你需要在这个函数中完成模型以及其他相关资源的加载，并调用inpaint进行图像补全，以生成图片
def test_image_inpainting():
    """
    测试图像补全功能。加载训练好的模型和原始图像，进行补全并保存结果。
    """
    # 配置参数
    device = "cuda" if torch.cuda.is_available() else "cpu"
    img_size = 64
    model_path = "./ddpm_mse_epochs_5000.pth"
    output_dir = "./inpainted_images"
    
    # 创建输出目录
    os.makedirs(output_dir, exist_ok=True)
    
    # 加载模型
    model = SimpleUnet()
    
    # 检查模型文件是否存在
    if not os.path.exists(model_path):
        print(f"模型文件 {model_path} 不存在，请先训练模型")
        return
    
    model.load_state_dict(torch.load(model_path, map_location=device))
    model.to(device)
    model.eval()
    
    print(f"使用设备: {device}")
    print("开始进行图像补全...")
    
    # 从数据集加载一张图像
    from dataloader import load_transformed_dataset
    dataloader = load_transformed_dataset(batch_size=1)
    
    for batch_idx, (img, _) in enumerate(dataloader):
        if batch_idx >= 2:  # 只处理前 2 张图像
            break
        
        img = img.to(device)
        
        # 创建掩码：中心区域需要补全
        mask = torch.ones_like(img)
        h, w = img_size // 4, img_size // 4
        mask[:, :, img_size//2-h:img_size//2+h, img_size//2-w:img_size//2+w] = 0
        mask = mask.to(device)
        
        # 保存原始图像
        original_path = os.path.join(output_dir, f"original_{batch_idx}.png")
        save_image(img, original_path)
        print(f"原始图像已保存到: {original_path}")
        
        # 创建带掩码的图像（掩码区域设为噪声）
        masked_img = img.clone()
        masked_img = masked_img * mask + torch.randn_like(masked_img) * (1 - mask)
        
        # 保存掩码图像
        masked_path = os.path.join(output_dir, f"masked_{batch_idx}.png")
        save_image(masked_img, masked_path)
        print(f"掩码图像已保存到: {masked_path}")
        
        # 进行补全
        print(f"补全第 {batch_idx+1} 张图像...")
        inpainted_img = inpaint(model, device, masked_img, mask, t_max=50)
        
        # 保存补全后的图像
        inpainted_path = os.path.join(output_dir, f"inpainted_{batch_idx}.png")
        save_image(inpainted_img, inpainted_path)
        print(f"补全图像已保存到: {inpainted_path}")
    
    print("图像补全完成！")
    

if __name__ == "__main__":
    # test_image_generation()
    test_image_inpainting()