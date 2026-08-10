# OptiScaler_chs

OptiScaler 简体中文汉化（全自动维护）

## 原理

本仓库通过 GitHub Actions 实现**全云端自动化**，本地零参与：

1. **定时拉上游**：每天 04:00 UTC 自动拉取 [optiscaler/OptiScaler](https://github.com/optiscaler/OptiScaler) 的 dev 分支（也可手动触发）
2. **注入汉化**：`scripts/apply_patch.py` 将汉化改造注入上游源码
   - 覆盖 `localization/`（LK 枚举 + 中英翻译表）
   - vcxproj 加 `/utf-8` 编译选项（修复中文乱码）
   - imgui.cpp FontStack 修复
   - menu_common.cpp 全量 LOC 注入 + CJK 字体加载
3. **自动翻译**：扫描上游新增 UI 字符串 → DeepSeek API 翻译（带术语表保证一致性）→ 自动提交回本仓库
4. **编译发布**：windows-latest runner 编译 → 打包 → 发 GitHub Release
   - `nightly-YYYYMMDD`：每日自动
   - `beta-YYYYMMDD` / `release-YYYYMMDD`：手动触发

## 目录结构

```
overlay/          # 汉化覆盖文件（localization 三件套等）
scripts/          # 自动化脚本
  apply_patch.py    # 汉化注入器
  scan_strings.py   # 新字符串扫描
  translate.py      # DeepSeek 自动翻译
  strings_map.json  # 英文串 → LK key 映射
  build.ps1         # Windows 构建
```

## 手动触发

Actions 页面 → `CHS Sync & Release` → Run workflow → 选择类型：
- `nightly`：每日预览
- `beta`：测试版
- `release`：正式版

## 依赖

- GitHub Secrets 需配置 `DEEPSEEK_API_KEY`（自动翻译用，可选，不配则只汉化已有字符串）
- 中文字体：构建时自动下载文泉驿微米黑（GPL + font embedding exception，可分发）

## 术语表（翻译一致性）

Upscaler=超分辨率、FG/Frame Generation=帧生成、Motion Vectors=运动矢量、Reactive Mask/HUDLess/DLSS/FSR/XeSS 等保留英文
