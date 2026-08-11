# 已知问题

## 游戏内菜单

如果无法打开游戏内菜单：
1. 请确认已在游戏选项中启用 DLSS、XeSS 或 FSR
2. 如果使用的是旧式安装方式，请在进入游戏后（正在进行 3D 渲染时）尝试打开菜单
3. 如果你在使用 RTSS（MSI Afterburner、CapFrameX），请启用 RTSS 的此设置并尝试更新 RTSS。
  ![image](https://github.com/optiscaler/OptiScaler/assets/35529761/8afb24ac-662a-40ae-a97c-837369e03fc7)

* 部分游戏不释放鼠标控制权，此时键盘和手柄控制仍应可用。
* 在某些系统和游戏组合下，打开旧版游戏内菜单可能导致游戏崩溃或图形损坏（尤其是在 Unreal Engine 5 游戏中）。

![Banishers](/images/banishers.png)<br>*《尘封大陆》（Banishers: Ghosts of New Eden）*

* 更改设置大多经过测试，但可能导致崩溃（尤其是更换后端或重新初始化后端时）。
* 使用 Unity 引擎旧版的游戏中，游戏内菜单会上下颠倒。

![barrel roll](/images/upsidedown.png)<br>*《森林之子》（Sons of Forest）*

## DirectX 11 配合 DirectX 12 超分辨率技术
此实现使用后台 DirectX12 设备，以使用仅限 DirectX12 的超分辨率技术。此方法有 10-15% 的性能损失，但提供了更多超分辨率选项。

## 曝光纹理（Exposure Texture）
有时游戏使用的曝光纹理格式无法被超分辨率技术识别。大多数情况下表现为颜色压暗（尤其是在黑暗区域）。

![exposure](/images/exposure.png)<br>*《古墓丽影：暗影》（Shadow of the Tomb Raider）*

大多数情况下，在 `OptiScaler.ini` 中启用 `AutoExposure=true`，或在游戏内菜单的 `Init Parameters` 中选择 `Auto Exposure` 即可修复这些问题。

## 资源屏障（Resource Barriers）
已知 Unreal Engine DLSS 插件会以错误状态发送 DLSS 资源。通常 OptiScaler 会从 NVSDK 检查引擎信息并自动为 Unreal Engine 游戏启用必要的修复，但部分游戏无法正确报告引擎信息。此问题通常表现为屏幕底部出现彩色区域。

![christmas lights](/images/christmas.png)<br>*《深岩银河》（Deep Rock Galactic）*

解决办法是从 `OptiScaler.ini` 设置 `ColorResourceBarrier=4`，或在游戏内菜单的 `Resource Barriers (Dx12)` 中为 `Color` 选择 `RENDER_TARGET`。

## XeSS 黑屏/花屏或崩溃
有用户反馈，使用 XeSS 超分辨率后端时会出现黑屏/花屏、仅显示 UI 或崩溃（例如《银河护卫队》）。某些情况下，下载最新版本的 [DirectX Shader Compiler](https://github.com/microsoft/DirectXShaderCompiler/releases)，并将 `bin\x64\` 中的 `dxcompiler.dll`、`dxil.dll` 解压到游戏 exe 旁边即可解决此问题。

## Minecraft RTX
XeSS 1.1 与 Minecraft RTX 的兼容性最好。但我也看到有报告说，通过[各种启动器](https://github.com/MCMrARM/mc-w10-version-launcher/releases)也可以使用 1.2 及以上版本。

## Linux 上的着色器编译错误
如果你在 Linux 上使用 OptiScaler，并且遇到 `RCAS`、`Reactive Mask Bias` 或 `Output Scaling` 的问题，你可能会在日志中看到这样的消息：
```
CompileShader error compiling shader : <anonymous>:83:26: E5005: Function "rcp" is not defined.
```
要解决此问题，可以使用菜单中的 `Precompiled Shaders` 选项，或使用 `WineTricks`/`ProtonTricks` 安装 `d3dcompiler_47`。OptiScaler 为这些功能使用自定义着色器，并在运行时依赖此编译器文件来编译这些着色器。

## 性能问题
* 一般来说，XeSS 对 GPU 的负担比 FSR 更重，因此即使在 Intel Arc 显卡上性能也更低，这是正常的。
* 由于伪装成 Nvidia 显卡以启用 DLSS，部分游戏会使用 Nvidia 优化过的代码路径，这可能导致其他 GPU 上性能更低。

## 显示分辨率运动矢量（Display Resolution Motion Vectors）
有时游戏会设置错误的 `DisplayResolution` 初始化标志，导致过度运动模糊。设置或重置 `DisplayResolution` 有助于解决此问题。

![mv wrong](/images/mv_wrong.png)<br>*《深岩银河》（Deep Rock Galactic）*

## 图形损坏与崩溃
如上所述，伪装成 Nvidia 显卡可能导致游戏使用特殊代码路径，从而造成图形损坏。如果可能，请在这些情况下禁用伪装并使用 FSR 或 XeSS 输入。

![talos principle 2](/images/talos.png)<br>*《塔洛斯法则 2》（Talos Principle 2）*

* 以及崩溃，尤其是在启用光线追踪时。
