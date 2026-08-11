<div align="center">

  ![Logo](https://github.com/user-attachments/assets/c7dad5da-0b29-4710-8a57-b58e4e407abd)

</div>
<hr />
<br />
<div align="center">
  <a href="https://github.com/sponsors/cdozdil?frequency=one-time"><img src="images/gh-sponsor-red.png" /></a>
  <a href="https://buymeacoffee.com/nitec"><img src="images/bmac.png" /></a>
</div>
<br />

## 目录

**1.** [**项目简介**](#项目简介)  
**2.** [**工作原理**](#工作原理)  
**3.** [**支持的 API 与超分辨率技术**](#支持的-api-与超分辨率技术)  
**4.** [**安装**](#安装)  
**5.** [**已知问题**](#已知问题)  
**6.** [**编译与致谢**](#编译)  
**7.** [**Wiki**](https://github.com/optiscaler/OptiScaler/wiki)

<br />
<div align="center">
  <a href="https://discord.gg/wEyd9w4hG5"><img src="https://img.shields.io/badge/OptiScaler-blue?style=for-the-badge&logo=discord&logoColor=white&logoSize=auto&color=5865F2" alt="Discord invite"></a>
  <a href="https://github.com/deYangar/OptiScaler_chs/releases/latest"><img src="https://img.shields.io/badge/Download-Stable-green?style=for-the-badge&logo=github&logoSize=auto" alt="Stable release"></a>
  <a href="https://github.com/deYangar/OptiScaler_chs/releases/tag/nightly"><img src="https://img.shields.io/badge/Download-Nightly-purple?style=for-the-badge&logo=github&logoSize=auto" alt="Nightly release"></a>
  <a href="https://github.com/optiscaler/OptiScaler/wiki"><img src="https://img.shields.io/badge/Documentation-blue?style=for-the-badge&logo=gitbook&logoColor=white&logoSize=auto" alt="Wiki"></a>
</div>

---

<div align="center">

# 🌏 OptiScaler 中文汉化版（OptiScaler_CHS）

<font size="6"><b>✨ 全中文界面 · 每日自动同步上游 · 开箱即用 ✨</b></font>

> 本项目是 [OptiScaler](https://github.com/optiscaler/OptiScaler) 的**中文汉化分支**，在保持上游功能完全一致的基础上，将游戏内菜单、界面文本及文档翻译为简体中文，并内置中文字体支持（文泉驿微米黑），开箱即用，无需额外配置。

</div>

<div align="center">

<table>
<tr>
<td style="background-color:#FFF8E1; border:2px solid #FFB300; border-radius:10px; padding:16px 32px;">

<font size="5"><b>🔗 本仓库与上游的关系</b></font>

| | |
|:---|---:|
| <font size="4">⬆️ **上游**</font> | <font size="4">[optiscaler/OptiScaler](https://github.com/optiscaler/OptiScaler)（原版，英文界面）</font> |
| <font size="4">🏠 **本仓库**</font> | <font size="4">每日自动同步上游源码 + 自动翻译新增文本 + 自动构建发布</font> |
| <font size="4">✅ **完全兼容**</font> | <font size="4">存档、配置文件格式不变，可直接替换 `dxgi.dll` / `nvngx.dll`</font> |

</td>
</tr>
</table>

</div>

---

## 项目简介

**OptiScaler** 是一款允许你在 ***已经支持 DLSS2+ / FSR2+ / XeSS*** 的游戏中替换超分辨率技术的工具（$`^1`$），同时也能管理上述游戏的***帧生成***功能（_既可以替换已有的帧生成选项，也可以通过实验性的 ***OptiFG*** 在 DX12 游戏中启用帧生成_）。它还为用户提供了丰富的自定义选项，包括使用 Nvidia 显卡 + DLSS 的用户。

> [!CAUTION]
> * 我们已得知一些**假冒网站**自称是 OptiScaler 团队，在此强调：我们**没有任何官方网站！**
> * 我们**没有官方的管理器应用**，请谨慎下载和使用！也请不要向我们索取与本项目无关的所谓"官方"支持！
> * 只有 **GitHub**、我们的 **Discord 服务器** 和 Nitec 的 **NexusMods 页面** 是**正规渠道**。
> * OptiScaler 是**免费的**，任何形式的收费要求都是诈骗！

> [!TIP]
> _例如：如果某游戏只有 DLSS，OptiScaler 可以用 XeSS 或 FSR 3.1 替换 DLSS（同样适用于只有 FSR2 的游戏，比如《天外世界：太空人之选》，不过需要手动提供 nvngx_dlss.dll）。_

**OptiScaler 的核心特性：**
- 在支持（时间）超分辨率的游戏中启用 XeSS、FSR2、FSR3、**FSR4**$`^2`$（_官方仅支持 RDNA4 和 RDNA3 独立显卡_）和 DLSS
- 通过大量微调与增强选项精细调整超分辨率体验（RCAS & MAS、输出缩放、DLSS Preset、比率 & DRS 覆盖等）
- 自 v0.7.0+ 起，加入***实验性 DX12*** 帧生成支持，并提供可能的 HUDfix 解决方案（[**OptiFG**](#optifg--hudfix实验性-hud-重影修复)）
- 支持 [**Fakenvapi**](#安装) 集成——可启用 Reflex 挂钩，并注入 _Anti-Lag 2_（仅限 RDNA1+）、_LatencyFlex_（LFX）或 _XeLL_ - _自 0.9 起内置_
- 自 v0.7.7 起，支持 **Nukem 的** FSR3-FG 模组 [**dlssg-to-fsr3**](#安装)，仅支持***原生 DLSS-FG*** 的游戏 - _自 0.9 起内置_
- 自 v0.7.8 起，支持 **ASI 插件加载**（_默认关闭_（`LoadAsiPlugins=` 于 INI），从可自定义文件夹加载，默认 `plugins`）
- 新项目 - [**OptiPatcher**](https://github.com/optiscaler/OptiPatcher) - 一个用于 OptiScaler 的 ASI 插件，可在***受支持的游戏***中无需伪装即可启用 DLSS 和 DLSSG 输入
- 自 v0.7.8 起，OptiScaler 会自动为某些游戏应用特定补丁，以获得更开箱即用的体验
- 自 v0.9.0 起，帧生成的输入与输出分离，新增 XeFG 和 FSR4-FG 支持，并内置 Fakenvapi 和 Nukem 的 FSR3-FG 模组
- 完整功能列表请查看 [Features.md](Features.md)（中文）

> [!IMPORTANT]
> _**始终查看 [Wiki 兼容性列表](https://github.com/optiscaler/OptiScaler/wiki)，了解已知游戏问题和解决方案。**_  
> 另外请查看文末的 [***OptiScaler 已知问题***](#已知问题)，了解 **RTSS** 兼容性。  
> 社区实测游戏可参考单独的 [***FSR4 兼容性列表***](https://github.com/optiscaler/OptiScaler/wiki/FSR4-Compatibility-List)。  
> ***[3]** 未内置的项目请查看 [安装](#安装)。*

> [!NOTE]
> ### 超分辨率技术说明
> <details>
>  <summary><b>点击展开 [1]、[2]</b></summary>  
>  
> **[1]** 对于 **Unreal Engine** 游戏，只有 UE XeSS -> Opti XeSS/FSR4 可用  
>  
> *关于 **XeSS** 输入：由于 **Unreal Engine 插件**不提供深度信息，替换游戏内 XeSS 会破坏其他超分辨率技术（例如 Redout 2 这类只有 XeSS 的游戏），但你仍然可以对 XeSS 应用 RCAS 锐化来减少画面模糊。*
> 
> *关于 **FSR 输入**：FSR 3.1 是第一个拥有完全标准化、面向未来的 API 的版本，应获得完整支持。由于 FSR2 和 FSR3 支持自定义接口，游戏支持程度取决于开发者的实现。对于 Unreal Engine 游戏，FSR 输入可能需要 [ini 调整](https://github.com/optiscaler/OptiScaler/wiki/Unreal-Engine-Tweaks)。*
>  
> **[2]** *关于 **FSR4**，请查看 [FSR4 兼容性列表](https://github.com/optiscaler/OptiScaler/wiki/FSR4-Compatibility-List) 了解已知受支持的游戏和一般信息。*
> 
> </details>

## 官方 Discord 服务器: [OptiScaler](https://discord.gg/wEyd9w4hG5)

*本项目基于 [PotatoOfDoom](https://github.com/PotatoOfDoom) 的杰出作品 [CyberFSR2](https://github.com/PotatoOfDoom/CyberFSR2)。*

## 工作原理
* OptiScaler 作为中间件，拦截游戏的超分辨率调用（_**输入**_）并将其重定向到所选的超分辨率后端（_**输出**_），从而允许用户用一种技术替换另一种技术。**输入 -> OptiScaler -> 输出**
* _更直白地说，**输入**是游戏设置中使用的超分辨率技术，**输出**是 Opti 覆盖层中选择的技术。_
* _帧生成选项同样分离为 **FG 输入** 和 **FG 输出**。_

> [!NOTE]
> * 在游戏中按 **`Insert`** 键应可打开 OptiScaler **覆盖层**，包含所有选项（_`ShortcutKey=` 可在 INI 文件中修改，或在覆盖层的 **按键绑定** 中修改_）。
> * 按 **`Page Up`** 在左上角显示性能统计覆盖层，可用 **`Page Down`** 在不同模式间切换（_按键绑定可在覆盖层中自定义_）。
> * 如果按几次 Insert 后 Opti 覆盖层立即消失，可以试试 **`Alt + Insert`**（[报告的解决方案](https://github.com/optiscaler/OptiScaler/issues/484)，适用于替代键盘布局）。

![inputs_and_outputs](https://github.com/user-attachments/assets/7ff37fd7-515f-488d-99ff-faa586e206fc)

## 支持的 API 与超分辨率技术
目前 **OptiScaler** 可用于 DirectX 11、DirectX 12 和 Vulkan，但每种 API 支持的超分辨率技术不同。  
[**OptiFG**](#optifg--hudfix实验性-hud-重影修复) 目前**仅支持 DX12**，将在单独段落说明。

#### DirectX 12
- XeSS（默认）
- FSR 2.1.2、2.2.1
- FSR 3.X（及 FSR 2.3.X）
- FSR 4.X（通过 FSR 3.X/4，_官方仅限 RDNA4 和 RDNA3 独立显卡_）
- DLSS

#### DirectX 11
- FSR 2.2.1（默认，原生 DX11）
- FSR 3.1.2（非官方移植到原生 DX11）
- DLSS（原生 DX11）
- XeSS 2.X（原生 DX11，_仅限 Intel ARC_）
- XeSS、FSR 2.1.2、2.2.1、FSR 3.X（通过 D3D11on12 使用 DX12）$`^1`$
- FSR 4.X（通过 FSR 3.X/4 的 DX12 互操作，_官方仅限 RDNA4 和 RDNA3 独立显卡_）

> [!NOTE]
> <details>
>  <summary><b>展开查看 [1]</b></summary>
>
> _**[1]** 这些实现使用后台 DirectX12 设备以使用仅限 DX12 的超分辨率技术。此方法有最高约 10% 的性能损失，但提供了更多超分辨率选项。此外，FSR 2.2.1 的原生 DX11 实现是从 Unity 渲染器移植而来，有其自身的问题，其中部分已由 OptiScaler 修复。_
> </details>

#### Vulkan
- FSR 4.X（通过 FSR 3.X/4 的 DX12 互操作，_官方仅限 RDNA4 和 RDNA3 独立显卡_）
- FSR2 2.1.2（默认）、2.2.1
- FSR3 3.1（及 FSR2 2.3.2）
- DLSS
- XeSS 2.x

#### OptiFG + HUDfix（实验性 HUD 重影修复）
**OptiFG** 自 **v0.7** 起加入，**仅支持 DX12**。  
它是一种为没有原生帧生成的游戏添加帧生成的**实验性**方法，也可以作为原生帧生成无法正常工作时最后的手段。  
* 目前支持 FSR3-FG（需要 HUDfix 以避免 HUD 重影）、XeFG 和 FSR4-FG（ML 模型处理 HUD，可能不一定需要 HUDfix）。

有关 OptiFG 及其用法的更多信息，请查看 Wiki 页面 - [OptiFG](https://github.com/optiscaler/OptiScaler/wiki/OptiFG)。

## 安装
> [!CAUTION]
> _**警告**：**请勿在联机游戏中使用此模组。** 可能触发反作弊软件并导致封号！_

> [!IMPORTANT]
> **安装步骤请查看 [**Wiki**](https://github.com/optiscaler/OptiScaler/wiki)**

> [!TIP]
> **汉化版快速使用：**
> 1. 从本仓库 Releases 下载最新 `OptiScaler_CHS_*.zip`
> 2. 将 `dxgi.dll`（或 `nvngx.dll`）放到游戏可执行文件旁边（与上游使用方法完全一致）
> 3. 中文字体已内置（`font/wqy-microhei.ttc`），游戏内菜单直接显示简体中文
> 4. 按 `Insert` 打开菜单，可在 **语言** 选项中切换中文/英文

## 配置
请查看 [Config.md](Config.md)（中文）了解配置参数和说明。如果你的显卡不是 Nvidia 的，请查看 [GPU 伪装选项](Spoofing.md)（中文）_（持续更新中）_

## 已知问题
> [!NOTE]
> **已知问题列表请查看 [**Wiki**](https://github.com/optiscaler/OptiScaler/wiki)**。
> 
> 也建议查看 [兼容性列表](https://github.com/optiscaler/OptiScaler/wiki/Compatibility-List)，了解可能的游戏问题及其修复方法。
> 
> 中文版参见 [Issues.md](Issues.md)（中文）。

## 编译

### 环境要求
* Visual Studio 2022

### 编译步骤
* 使用**所有子模块**克隆本仓库。
* 用 Visual Studio 2022 打开 OptiScaler.sln。
* 构建项目。

> [!NOTE]
> **汉化注入说明**：本仓库的汉化通过 `scripts/apply_patch.py` 在构建时自动注入（自动添加 `/utf-8` 编译选项、中文字体加载和 localization 编译单元），无需手动干预。CI 会自动完成同步上游 → 注入 → 编译 → 发布的全流程。

## 致谢
* @PotatoOfDoom 的 CyberFSR2
* @Artur 的 DLSS Enabler，以及帮助正确实现 NVNGX API
* @LukeFZ 与 @Nukem 的优秀模组与知识分享
* @FakeMichau 的持续支持、测试与功能贡献
* @QM 的持续测试工作，帮助接触到更多游戏
* @TheRazerMD 的持续测试与支持
* @Cryio、@krispy、@krisshietala、@Lordubuntu、@scz、@Veeqo 在（现已过时的）[兼容性矩阵](https://docs.google.com/spreadsheets/d/1qsvM0uRW-RgAYsOVprDWK2sjCqHnd_1teYAx00_TwUY)上的辛勤工作
* 以及整个 DLSS2FSR 社区的支持

## 许可
本项目使用 [FreeType](https://gitlab.freedesktop.org/freetype/freetype)，遵循 [FTL](https://gitlab.freedesktop.org/freetype/freetype/-/blob/master/docs/FTL.TXT) 许可。
内置中文字体 [文泉驿微米黑](https://launchpad.net/wqy)（wqy-microhei）遵循 GPL 许可，可自由再分发。

