import torch
import torch.nn.functional as F
import matplotlib.pyplot as plt
from dataloader import load_transformed_dataset, show_tensor_image


def linear_beta_schedule(timesteps, start=0.0001, end=0.02):
    """
    生成线性的噪声调度（Beta Schedule）。
    
    在扩散模型中，Beta 控制每一步添加的噪声量。线性调度意味着噪声量从 start 线性增加到 end。
    
    参数：
        timesteps (int): 总的时间步数，通常为 1000 或更大
        start (float): 初始的 beta 值，默认 0.0001（第一步添加很少的噪声）
        end (float): 最终的 beta 值，默认 0.02（最后一步添加较多的噪声）
    
    返回：
        torch.Tensor: 形状为 (timesteps,) 的张量，包含每个时间步的 beta 值
    
    示例：
        betas = linear_beta_schedule(timesteps=1000)  # 返回 1000 个 beta 值
    """
    return torch.linspace(start, end, timesteps)


def get_index_from_list(vals, time_step, x_shape):
    """
    从预计算的值列表中根据时间步索引获取对应的值，并调整形状以适应批处理。
    
    这个函数用于在扩散过程中快速查找特定时间步的预计算参数（如 alpha、beta 等）。
    它会自动处理批维度，使得返回的值可以直接与图像张量进行广播运算。
    
    参数：
        vals (torch.Tensor): 预计算的值列表，形状为 (timesteps,)，例如 alphas_cumprod
        time_step (torch.Tensor): 时间步索引，形状为 (batch_size,)，值范围 [0, timesteps-1]
        x_shape (tuple): 输入图像的形状，例如 (batch_size, channels, height, width)
    
    返回：
        torch.Tensor: 形状为 (batch_size, 1, 1, 1) 的张量，包含对应时间步的值
                     这样的形状便于与图像张量进行广播运算
    
    示例：
        alphas_cumprod = torch.tensor([0.99, 0.98, 0.97, ...])
        time_step = torch.tensor([0, 2, 1])  # 批大小为 3
        x_shape = (3, 3, 64, 64)  # 3 张 RGB 图像，64x64 像素
        result = get_index_from_list(alphas_cumprod, time_step, x_shape)
        # result.shape = (3, 1, 1, 1)，可以直接与 x 相乘
    """
    batch_size = time_step.shape[0]
    out = vals.gather(-1, time_step.cpu())
    return out.reshape(batch_size, *((1,) * (len(x_shape) - 1))).to(time_step.device)




# ============ 扩散过程的预计算参数 ============
# 这些参数在训练和推理前计算一次，然后在整个过程中重复使用，以提高效率

# 定义扩散过程的总时间步数
T = 300

# 生成线性 beta 调度：从 0.0001 到 0.02，共 300 步
# beta_t 表示第 t 步添加的噪声方差
betas = linear_beta_schedule(timesteps=T)

# alpha_t = 1 - beta_t
# 表示保留原始信号的比例（1 - 噪声比例）
alphas = 1.0 - betas

# alpha_cumprod_t = ∏(alpha_i) for i=0 to t
# 累积乘积，表示从 t=0 到 t 步保留的原始信号比例
alphas_cumprod = torch.cumprod(alphas, axis=0)

# alpha_cumprod_{t-1}，用于计算后验方差
# 在 t=0 时补充 1.0，因为没有 t=-1 步
alphas_cumprod_prev = F.pad(alphas_cumprod[:-1], (1, 0), value=1.0)

# sqrt(1 / alpha_t)，用于缩放噪声
sqrt_recip_alphas = torch.sqrt(1.0 / alphas)

# sqrt(alpha_cumprod_t)，用于计算前向扩散中原始信号的系数
sqrt_alphas_cumprod = torch.sqrt(alphas_cumprod)

# sqrt(1 - alpha_cumprod_t)，用于计算前向扩散中噪声的系数
sqrt_one_minus_alphas_cumprod = torch.sqrt(1.0 - alphas_cumprod)

# 后验方差：用于反向扩散过程中的方差计算
# posterior_variance_t = beta_t * (1 - alpha_cumprod_{t-1}) / (1 - alpha_cumprod_t)
posterior_variance = betas * (1.0 - alphas_cumprod_prev) / (1.0 - alphas_cumprod)



# TODO: 你需要完善这个函数，以实现对输入图像的加噪过程
def forward_diffusion_sample(x_0, time_step, device="cpu"):
    noise = torch.randn_like(x_0)  
    sqrt_alphas_cumprod_t = get_index_from_list(sqrt_alphas_cumprod,time_step,x_0.shape)
    sqrt_one_minus_alphas_cumprod_t = get_index_from_list(sqrt_one_minus_alphas_cumprod,time_step,x_0.shape)
    x_t = sqrt_alphas_cumprod_t*x_0 + sqrt_one_minus_alphas_cumprod_t*noise  
    return x_t, noise
