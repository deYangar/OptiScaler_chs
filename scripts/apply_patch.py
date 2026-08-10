#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
OptiScaler 汉化补丁注入器
用法: python apply_patch.py <上游源码根目录>
把汉化改造注入到上游最新源码：
1. 覆盖 localization 目录（我们的 LK enum + 中英翻译表）
2. vcxproj 加 /utf-8 编译选项
3. imgui.cpp NewFrame 强制清空 FontStack（修复中文渲染）
4. menu_common.cpp: include localization + 全部字符串 LOC 注入 + CJK 字体加载
"""
import io
import json
import os
import re
import shutil
import sys

UPSTREAM = os.path.abspath(sys.argv[1])
HERE = os.path.dirname(os.path.abspath(__file__))
OVERLAY = os.path.normpath(os.path.join(HERE, '..', 'overlay'))
FONTS = os.path.normpath(os.path.join(HERE, '..', 'fonts'))

OS_DIR = os.path.join(UPSTREAM, 'OptiScaler')
LOCAL_DIR = os.path.join(OS_DIR, 'localization')
MC = os.path.join(OS_DIR, 'menu', 'menu_common.cpp')
VCX = os.path.join(OS_DIR, 'OptiScaler.vcxproj')
IMGUI = os.path.join(OS_DIR, 'include', 'imgui', 'imgui.cpp')

def read(p):
    with io.open(p, encoding='utf-8') as f:
        return f.read()

def write(p, c):
    with io.open(p, 'w', encoding='utf-8', newline='\n') as f:
        f.write(c)

print(f'[1] 覆盖 localization/')
os.makedirs(LOCAL_DIR, exist_ok=True)
for fn in os.listdir(os.path.join(OVERLAY, 'OptiScaler', 'localization')):
    shutil.copy2(os.path.join(OVERLAY, 'OptiScaler', 'localization', fn), os.path.join(LOCAL_DIR, fn))
    print(f'    {fn}')

print(f'[2] vcxproj /utf-8')
vcx = read(VCX)
if '/utf-8' not in vcx:
    n = vcx.count('<AdditionalOptions>/w34996 %(AdditionalOptions)</AdditionalOptions>')
    vcx = vcx.replace('<AdditionalOptions>/w34996 %(AdditionalOptions)</AdditionalOptions>',
                      '<AdditionalOptions>/w34996 /utf-8 %(AdditionalOptions)</AdditionalOptions>')
    write(VCX, vcx)
    print(f'    注入 /utf-8 x{n}')
else:
    print('    已存在')

print(f'[3] imgui.cpp FontStack fix')
imgui = read(IMGUI)
if 'FIX-CJK' not in imgui:
    anchor = '    SetupDrawListSharedData();\n    PushDefaultFont();'
    if imgui.count(anchor) != 1:
        print('    !! 锚点未找到，FontStack fix 注入失败')
        sys.exit(2)
    imgui = imgui.replace(anchor,
        '    SetupDrawListSharedData();\n'
        '    // FIX-CJK: 强制清空字体栈，保证 PushDefaultFont 的 Size==1 条件成立、SetCurrentFont 总会执行。\n'
        '    g.FontStack.clear();\n'
        '    PushDefaultFont();')
    write(IMGUI, imgui)
    print('    已注入')
else:
    print('    已存在')

print(f'[4] menu_common.cpp LOC 注入')
mc = read(MC)
if '#include <localization/localization.h>' not in mc:
    # 在第一个 #include 后插入
    first_inc = mc.find('#include')
    mc = mc[:first_inc] + '#include <localization/localization.h>\n' + mc[first_inc:]
    print('    注入 include')

with io.open(os.path.join(HERE, 'strings_map.json'), encoding='utf-8') as f:
    strings_map = json.load(f)

injected = 0
for en, key in strings_map.items():
    old = '"' + en + '"'
    if old in mc:
        mc = mc.replace(old, 'LOC(' + key + ')')
        injected += 1
write(MC, mc)
print(f'    LOC 注入 {injected} 处')

print(f'[5] menu_common.cpp CJK 字体加载')
if 'cjk_ranges' not in mc:
    cjk_block = '''
        // FIX-CJK: 中文字体加载（覆盖 Latin + CJK）
        {
            static const ImWchar cjk_ranges[] = {
                0x0020, 0x00FF, 0x2000, 0x206F, 0x3000, 0x30FF,
                0xFF00, 0xFFEF, 0x4E00, 0x9FFF, 0,
            };
            auto dllDir = Util::DllPath().parent_path();
            std::vector<std::filesystem::path> fontPaths = {
                dllDir / "font" / "SarasaGothicSC-Regular.ttf",
                dllDir / "font" / "wqy-microhei.ttc",
                dllDir / "font" / "NotoSansSC-Regular.ttf",
            };
            for (const auto& cjkPath : fontPaths) {
                if (std::filesystem::exists(cjkPath)) {
                    ImFontConfig cfg{};
                    ImFont* cjkFont = atlas->AddFontFromFileTTF(wstring_to_string(cjkPath.wstring()).c_str(),
                                                                fontSize, &cfg, cjk_ranges);
                    if (cjkFont) {
                        io.FontDefault = cjkFont;
                        LOG_INFO("CHS: CJK font loaded from {}", cjkPath.filename().string());
                    }
                    break;
                }
            }
        }
'''
    # 锚点：UseHQFont 分支的 AddFont 块结束后插入
    # 找 "io.FontDefault = atlas->AddFontFromMemoryCompressedBase85TTF" 所属 else 块的结束
    anchors = [
        'io.FontDefault = atlas->AddFontFromMemoryCompressedBase85TTF(hack_compressed_compressed_data_base85,',
    ]
    found = False
    for a in anchors:
        idx = mc.find(a)
        if idx >= 0:
            # 找这个语句后的第一个 '}'（else 块结束）
            close = mc.find('}', idx)
            if close >= 0:
                mc = mc[:close] + cjk_block + mc[close:]
                found = True
                break
    if not found:
        print('    !! CJK 字体加载锚点未找到，请人工检查')
        sys.exit(3)
    write(MC, mc)
    print('    已注入 CJK 字体加载')
else:
    print('    已存在')

print(f'[6] 字体文件')
os.makedirs(os.path.join(OS_DIR, '..', 'run', 'font'), exist_ok=True)
font_dst = os.path.join(UPSTREAM, 'font')
os.makedirs(font_dst, exist_ok=True)
if os.path.isdir(FONTS):
    for fn in os.listdir(FONTS):
        if fn.lower().endswith(('.ttf', '.ttc')):
            shutil.copy2(os.path.join(FONTS, fn), os.path.join(font_dst, fn))
            print(f'    字体 {fn}')

print('\n补丁注入完成 ✅')
