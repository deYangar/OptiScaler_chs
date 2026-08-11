# 功能特性

* 支持多种超分辨率后端（XeSS、FSR 2.1.2、FSR 2.2.1、FSR 3.1 和 DLSS）
* 自 v0.7.0 起实验性支持帧生成（基于 FSR 的 OptiFG）
* 支持 DLSS 3.7 及以上版本（查看[安装说明](#以非-nvngx-方式安装)）
* 支持 Nvidia 显卡上的 DLSS-D（光线重建），（支持更改预设并使用 OptiScaler 增强功能）
* 支持在游戏运行时修改 DLSS/DLSS-D 预设
* 支持 XeSS v1.3.x 的超级性能、NativeAA 模式（**不使用默认的 XeSS 1.3.x 缩放比例，而是使用旧的比例**）
* 内置[游戏内菜单](https://github.com/optiscaler/OptiScaler/blob/master/Config.md)，可实时调整并保存设置（快捷键为 **INSERT**）
* 与 [DLSS Enabler](https://www.nexusmods.com/site/mods/757) 完全集成，支持 DLSS-FG
* 所有 Dx12 与 Dx11 超分辨率技术均支持 **RCAS** 锐化和 **MAS**（运动自适应锐化）
* 支持 **输出缩放** 选项（0.5x 至 3.0x），适用于运行在 Dx12 与 Dx11 上的后端
* 支持 DXGI 伪装（以 `dxgi.dll` 运行时），可将 GPU 伪装为 Nvidia 显卡（带 XeSS 检测，可在 Intel Arc 显卡上启用 XMX）
* 支持 Vulkan 伪装（需从 `nvngi.ini` 启用），可将 GPU 伪装为 Nvidia 显卡（不适用于《毁灭战士：永恒》）
* 支持加载指定的 `nvapi64.dll` 文件（以非 nvngx 模式运行时）
* 支持加载指定的 `nvngx_dlss.dll` 文件（以非 nvngx 模式运行时）
* 支持覆盖缩放比例
* 支持覆盖 DRS 范围
* 自动修复 Unreal Engine 与 AMD 显卡上的[彩色灯光](https://github.com/optiscaler/OptiScaler/blob/master/Config.md#resource-barriers-dx12-only)问题
* 自动修复[曝光纹理缺失](https://github.com/optiscaler/OptiScaler/blob/master/Config.md#init-flags)问题
* 支持修改游戏中的 [Mipmap Lod Bias](https://github.com/optiscaler/OptiScaler/blob/master/Config.md#mipmap-lod-bias-override-dx12-only) 值
* 支持 [Fakenvapi](https://github.com/FakeMichau/fakenvapi) 集成，可启用 Reflex 挂钩并注入 Anti-Lag 2 或 LatencyFlex（LFX）
* 支持 Nukem 的 FSR FG 模组 [dlssg-to-fsr3](https://github.com/Nukem9/dlssg-to-fsr3)（自 v0.7.7 起）

**为了绕过 DLSS 3.7 的签名检查要求，OptiScaler 使用了由 **Artur**（[DLSS Enabler](https://www.nexusmods.com/site/mods/757?tab=description) 的作者）开发的方法。**
