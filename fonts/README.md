# 中文字体

构建时自动从 Debian 源下载**文泉驿微米黑**（`wqy-microhei.ttc`，GPL + font embedding exception，可再分发）。

如需更换字体（如更纱黑体 Sarasa Gothic），修改 workflow 中的 `FONT_DEB` 环境变量或手动放置字体到构建目录的 `font/` 下，并同步调整 `scripts/apply_patch.py` 中的 `fontPaths`。
