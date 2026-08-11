# GPU 伪装（Spoofing）

除第一代 DLSS2 游戏外，其余游戏都有某种 Nvidia 验证机制，用于决定是否启用 DLSS 选项。
为了绕过这些检查，模组作者开发了一些工具。

## Windows

### Nvapi
对于 Nvapi 调用伪装，可以使用 FakeNvapi。某些游戏（如《古墓丽影：暗影》等）需要它才能启用 DLSS 支持。

同时作为**额外福利**，最新版本的 FakeNvapi 增加了对 AMD 的 AntiLag 2 和 LatencyFlex 的支持，可在支持 Nvidia Reflex 的游戏中降低输入延迟。

##### 使用方法
只需将 `nvapi64.dll` 放在 OptiScaler 旁边，并在 `OptiScaler.ini` 中设置 `OverrideNvapiDll=true`。此方法仅在 OptiScaler 以非 nvngx 模式（即不是 `nvngx.dll`）运行时有效。

不使用 OptiScaler 时的用法：
你需要将 `nvapi64.dll` 文件放到 `%WINDIR%\System32` 目录，但**请小心！**
* 如果你是 Nvidia 用户，请**备份原始文件**，并在模组使用结束后恢复。
* 请勿在联机游戏中使用此模组，可能触发反作弊问题或导致封号。

##### 链接
[FakeNvapi](https://github.com/FakeMichau/fakenvapi/releases)

### DXGI
OptiScaler 内置 DXGI 伪装选项，在以非 nvngx 模式（即不是 `nvngx.dll`）运行时默认启用。

#### d3d12-proxy
另外，对于 DXGI 适配器检查的伪装，可以使用 d3d12-proxy。该模组会将你的 GPU 报告为 RTX 4090。

##### 使用方法
只需将 dxgi.dll 文件放在游戏可执行文件旁边。

##### 链接
[d3d12-proxy](https://github.com/cdozdil/d3d12-proxy/releases)

### Vulkan
OptiScaler 在以非 nvngx 模式（即不是 `nvngx.dll`）运行时内置 Vulkan 伪装选项。
Vulkan 伪装默认禁用，需要时从 `OptiScaler.ini` 启用。

```ini
; 启用 Vulkan 的 Nvidia GPU 伪装
; true 或 false - 默认 (auto) 为 false
Vulkan=auto

; 启用 Vulkan 的 Nvidia 扩展伪装
; true 或 false - 默认 (auto) 为 false
VulkanExtensionSpoofing=auto
```

#### vulkan-spoofer
另外，对于 `GetPhysicalDeviceProperties` 检查的伪装，可以使用 vulkan-spoofer。该模组会将你的 GPU 报告为 RTX 4090。
兼容性时好时坏，适用于《无人深空》（不适用于最新的 streamline 补丁），但不适用于《毁灭战士：永恒》。

##### 使用方法
只需将 version.dll 文件放在游戏可执行文件旁边。

##### 链接
[vulkan-spoofer](https://github.com/cdozdil/vulkan-spoofer/releases)

## Linux
在 Linux 上，你可以使用 Wine 和 DXVK 内置的伪装机制。

### DirectX 与 Vulkan
对于 DXGI 和 Vulkan 伪装，只需在游戏可执行文件旁边创建一个 `dxvk.conf` 文件，内容如下，或从[此处](https://raw.githubusercontent.com/cdozdil/CyberXeSS/imgui-intergration/dxvk.conf)下载。

```ini
dxgi.customVendorId = 10de
dxgi.hideAmdGpu = True
dxgi.hideNvidiaGpu = False
dxgi.customDeviceId = 2684
dxgi.customDeviceDesc = "NVIDIA GeForce RTX 4090"
```

### NVAPI
要在 Proton 下伪装 NVAPI，请设置环境变量 `PROTON_FORCE_NVAPI=1`。

## Goghor 的 DLSS 解锁器
Goghor 为许多游戏制作了 DLSS 解锁器模组，可在他的 [Nexus](https://www.nexusmods.com/spidermanmilesmorales/users/12564231?tab=user+files&BH=0) 主页找到。
例如，据我所知，《毁灭战士：永恒》目前仍然只能通过他的模组启用 DLSS。
