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

# Windows 控制台默认 cp1252，强制 UTF-8 输出避免 UnicodeEncodeError
if hasattr(sys.stdout, 'reconfigure'):
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    sys.stderr.reconfigure(encoding='utf-8', errors='replace')
else:
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

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

print(f'[2] vcxproj /utf-8 + localization.cpp')
vcx = read(VCX)
if '/utf-8' not in vcx:
    n = vcx.count('<AdditionalOptions>/w34996 %(AdditionalOptions)</AdditionalOptions>')
    vcx = vcx.replace('<AdditionalOptions>/w34996 %(AdditionalOptions)</AdditionalOptions>',
                      '<AdditionalOptions>/w34996 /utf-8 %(AdditionalOptions)</AdditionalOptions>')
    write(VCX, vcx)
    print(f'    注入 /utf-8 x{n}')
else:
    print('    /utf-8 已存在')

# 确保 localization.cpp 被编译（vcxproj 需显式引用，否则链接器找不到 LocalizationManager::Get）
if 'localization\\localization.cpp' not in vcx and 'localization/localization.cpp' not in vcx:
    anchor = '<ClCompile Include="menu\\menu_common.cpp" />'
    if anchor in vcx:
        vcx = vcx.replace(anchor,
                          '<ClCompile Include="localization\\localization.cpp" />\n    ' + anchor)
        write(VCX, vcx)
        print('    注入 localization.cpp 编译项')
    else:
        print('    !! menu_common.cpp 锚点未找到，localization.cpp 未注入')
else:
    print('    localization.cpp 已存在')

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

with io.open(os.path.join(HERE, 'strings_map.json'), encoding='utf-8') as f:
    strings_map = json.load(f)

print(f'[4.1] menu_common.cpp splashText -> splashKeys (LK 枚举) 转换')
mc = read(MC)
# splashText 是文件作用域 static，在 main() 之前初始化，此时 LocalizationManager::Init() 尚未调用
# -> LOC() 全部返回 [MISSING]。改为存 LK 枚举，显示时才调 LOC()。
# 必须在 4.2/4.5/5 之前执行，否则字符串被替换成 LOC() 后无法匹配原始文本
splash_decl_pat = re.compile(
    r'(static\s+std::vector<std::string>\s+splashText\s*=\s*\{)(.*?)(\};)',
    re.DOTALL
)
m = splash_decl_pat.search(mc)
if m:
    block = m.group(2)
    strs = re.findall(r'"((?:[^"\\]|\\.)*)"', block)
    # strings_map 里转义不匹配的 3 个，手动补
    splash_override = {
        "MFG totally works with Nukem's 100%% no scam": "Splash_279",
        'Even supports \\"software\\" XeSS!': 'Splash_282',
        '\\"Framegen really attracts some strange clientelle\\"': 'Splash_329',
    }
    lk_list = []
    missing_count = 0
    for s in strs:
        raw = s.replace('\\"', '"').replace("\\'", "'").replace('%%', '%')
        if raw in strings_map:
            lk_list.append('LK::' + strings_map[raw])
        elif s in strings_map:
            lk_list.append('LK::' + strings_map[s])
        elif s in splash_override:
            lk_list.append('LK::' + splash_override[s])
        elif raw in splash_override:
            lk_list.append('LK::' + splash_override[raw])
        else:
            missing_count += 1
            print(f'    !! 未找到映射: {repr(s)} / {repr(raw)}')
            lk_list.append('LK::Splash_253')
    new_block = '\n' + ',\n'.join('        ' + x for x in lk_list) + '\n    '
    mc = mc[:m.start()] + 'static std::vector<LK> splashKeys = {' + new_block + '};' + mc[m.end():]
    print(f'    转换 {len(lk_list)} 个 splash 字符串为 LK 枚举')
    if missing_count:
        print(f'    !! {missing_count} 个未找到映射，已用 fallback')
else:
    print('    !! splashText 声明未找到，跳过')
    sys.exit(5)
mc = mc.replace('splashText[std::rand() % splashText.size()]',
                'LocalizationManager::Instance().Get(splashKeys[std::rand() % splashKeys.size()])')
write(MC, mc)
print('    splashText 引用已替换为 splashKeys + Get()')

print(f'[4.2] 多段拼接字符串链整体 LOC 化（结构体数组/跨行拼接，整链命中 strings_map 才替换）')
mc = read(MC)
chain_pat = re.compile(r'"((?:[^"\\]|\\.)*)"(?:\s*"((?:[^"\\]|\\.)*)")+')
chain_count = 0
def chain_repl(m):
    global chain_count
    full = ''.join(re.findall(r'"((?:[^"\\]|\\.)*)"', m.group(0)))
    key = strings_map.get(full)
    if key:
        chain_count += 1
        return 'LOC(' + key + ')'
    return m.group(0)
mc = chain_pat.sub(chain_repl, mc)
write(MC, mc)
print(f'    拼接链整体替换 {chain_count} 处')

print(f'[4.5] menu_common.cpp tooltip LOC 化（硬编码 ShowHelpMarker 字符串 → LOC(key)）')
mc = read(MC)

def parse_string_literal(src, start):
    """解析 C++ 字符串字面量（含相邻拼接），返回 (完整文本, 结束位置)"""
    i = start
    segs = []
    while i < len(src) and src[i] == '"':
        i += 1
        buf = []
        while i < len(src):
            c = src[i]
            if c == '\\':
                buf.append(src[i:i+2]); i += 2
                continue
            if c == '"':
                i += 1
                break
            buf.append(c); i += 1
        segs.append(''.join(buf))
        j = i
        while j < len(src) and src[j] in ' \t\r\n':
            j += 1
        if j < len(src) and src[j] == '"':
            i = j
            continue
        break
    return ''.join(segs), i

with io.open(os.path.join(HERE, 'strings_map.json'), encoding='utf-8') as f:
    strings_map = json.load(f)

tt_count = 0
pos = 0
# 显示函数列表（多行字符串被 LOC 注入跳过规则漏掉的，这里统一补上；strings_map 命中才替换）
DISPLAY_CALLS = ['ShowHelpMarker(', 'ShowTooltip(', 'ImGui::Text(', 'ImGui::TextDisabled(',
                 'ImGui::TextColored(', 'ImGui::BulletText(', 'ImGui::TextWrapped(',
                 'ImGui::SetItemTooltip(', 'SetItemTooltip(', 'SeparatorWithHelpMarker(']
while True:
    # 找最近的一个显示函数调用
    idx = -1
    fn_used = ''
    for fn in DISPLAY_CALLS:
        i = mc.find(fn, pos)
        if i != -1 and (idx == -1 or i < idx):
            idx = i
            fn_used = fn
    if idx == -1:
        break
    j = idx + len(fn_used)
    while j < len(mc) and mc[j] in ' \t\r\n':
        j += 1
    if j < len(mc) and mc[j] == '"':
        text, end = parse_string_literal(mc, j)
        if text in strings_map:
            key = strings_map[text]
            # 检查字符串结束后的字符：')'=纯字符串调用（吃括号）；','=带参数调用（保留参数）
            k = end
            while k < len(mc) and mc[k] in ' \t\r\n':
                k += 1
            if k < len(mc) and mc[k] == ')':
                repl = fn_used[:-1] + '(LOC(' + key + '))'
                end2 = k + 1
            else:
                repl = fn_used[:-1] + '(LOC(' + key + ')'
                end2 = end
            mc = mc[:idx] + repl + mc[end2:]
            tt_count += 1
            pos = idx + len(repl)
            continue
    pos = idx + len(fn_used)

# 阶段 2：行首引号的行（步骤 5 的"续行跳过"规则误伤的数组元素字符串等）
# 只处理独立字符串值行：行尾以 , 或 } 结尾，且上一行不是未闭合字符串（排除拼接链续段）
import re as _re
new_lines = []
prev_ends_with_quote = False
for line in mc.split('\n'):
    stripped_line = line.rstrip()
    is_cont = prev_ends_with_quote  # 上一行以引号结尾（未闭合）→ 本行是拼接续段
    if stripped_line.endswith(('}', ',')) and not is_cont:
        m = _re.match(r'^(\s*)"((?:[^"\\]|\\.)*)"(\s*[,}]*\s*)$', line)
        if m and m.group(2) in strings_map:
            key = strings_map[m.group(2)]
            line = m.group(1) + 'LOC(' + key + ')' + (m.group(3) or '')
            tt_count += 1
    # 更新续段标记：本行以引号结尾且该引号前无终止符（字符串未闭合，有续行）
    prev_ends_with_quote = stripped_line.endswith('"') and not stripped_line.endswith(('"}', '",', '");', '";'))
    new_lines.append(line)
mc = '\n'.join(new_lines)
write(MC, mc)
print(f'    tooltip LOC 化 {tt_count} 处')

print(f'[5] menu_common.cpp LOC 注入')
mc = read(MC)
LOC_INC = '#include <localization/localization.h>'
if LOC_INC not in mc:
    # 插到 pch.h 之后（预编译头模式下 pch.h 之前的内容会被 MSVC 静默忽略）
    pch_anchor = '#include "pch.h"'
    if pch_anchor in mc:
        idx = mc.find(pch_anchor) + len(pch_anchor)
        mc = mc[:idx] + '\n' + LOC_INC + mc[idx:]
        print('    注入 include（pch.h 后）')
    else:
        first_inc = mc.find('#include')
        nl = mc.find('\n', first_inc)
        mc = mc[:nl + 1] + LOC_INC + '\n' + mc[nl + 1:]
        print('    注入 include（首个 include 后）')
elif LOC_INC in mc and mc.find(LOC_INC) < mc.find('#include "pch.h"'):
    # include 已存在但在 pch.h 之前（旧版注入残留/缓存残留）→ 移到 pch.h 后
    mc = mc.replace(LOC_INC + '\n', '')
    mc = mc.replace(LOC_INC, '')
    pch_anchor = '#include "pch.h"'
    idx = mc.find(pch_anchor) + len(pch_anchor)
    mc = mc[:idx] + '\n' + LOC_INC + mc[idx:]
    print('    修复 include 位置（移到 pch.h 后）')

with io.open(os.path.join(HERE, 'strings_map.json'), encoding='utf-8') as f:
    strings_map = json.load(f)

injected = 0
# 按行注入：跳过 #include 行，防止把 include 文件名误替换成 LOC()
# 同时跳过：std::format 调用行（格式串需编译期常量）、跨行字符串拼接（行首"或行尾引号后无终止符）
new_lines = []
prev_dangling = False
lines_list = mc.split('\n')
for li, line in enumerate(lines_list):
    ls = line.lstrip()
    stripped = line.rstrip()
    is_cont = prev_dangling
    # 本行是否以未闭合字符串结尾（结尾引号后无终止符）→ 下一行是拼接续段
    prev_dangling = stripped.endswith('"') and not re.search(r'"[),;}]\s*$', stripped)
    if ls.startswith('#include'):
        new_lines.append(line)
        continue
    # std::format / fmt::format 的格式串必须是编译期常量，LOC() 是运行时调用 → 跳过整行
    if re.search(r'\b(?:std|fmt)::(?:v?format|format_to)\s*\(', line):
        new_lines.append(line)
        continue
    # LOG_TRACE/DEBUG/INFO/WARN/ERROR/TRACK 等宏在 0.9.x 树上定义为 __FUNCTION__ " " msg 字面量拼接，
    # 不接受运行时 LOC() 调用（master 树可以但为保持日志英文统一也跳过）→ 跳过整行
    if re.search(r'\bLOG_(?:TRACE|DEBUG|DEBUG_ONLY|DEBUG_ASYNC|INFO|WARN|ERROR|TRACK)\s*\(', line):
        new_lines.append(line)
        continue
    # 真·跨行字符串续行：行首是引号且上一行字符串未闭合 → 跳过
    # （多行数组初始化器如 `"Quality",` 上一行已闭合，不属于续行，不跳过）
    if ls.startswith('"') and is_cont:
        new_lines.append(line)
        continue
    # 行尾字符串未闭合且下一非空行以引号开头 = 拼接链的一段 → 跳过，防止把链拦腰截断
    # （整链翻译由 [4.2] 阶段处理；下一行不是引号则说明是完整字面量如三元表达式跨行参数，允许替换）
    if prev_dangling:
        j = li + 1
        while j < len(lines_list) and lines_list[j].strip() == '':
            j += 1
        if j < len(lines_list) and lines_list[j].lstrip().startswith('"'):
            new_lines.append(line)
            continue
    for en, key in strings_map.items():
        old = '"' + en + '"'
        if old in line:
            line = line.replace(old, 'LOC(' + key + ')')
            injected += 1
    new_lines.append(line)
mc = '\n'.join(new_lines)
write(MC, mc)
print(f'    LOC 注入 {injected} 处')

# 注入 LocalizationManager::Init() 调用（MenuCommon::Init 开头），否则表格未初始化、LOC 全部返回 [MISSING]
INIT_MARK = 'LocalizationManager::Instance().Init();'
if INIT_MARK not in mc:
    init_anchor = 'void MenuCommon::Init(HWND InHwnd, bool isUWP)\n{'
    if init_anchor in mc:
        idx = mc.find(init_anchor) + len(init_anchor)
        mc = mc[:idx] + '\n    ' + INIT_MARK + '\n' + mc[idx:]
        write(MC, mc)
        print('    注入 LocalizationManager::Init()（MenuCommon::Init 开头）')
    else:
        print('    !! MenuCommon::Init 锚点未找到，Init 注入失败')
        sys.exit(4)
else:
    print('    Init 已存在')

print(f'[6] menu_common.cpp CJK 字体加载')
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

print(f'[7] 字体文件')
os.makedirs(os.path.join(OS_DIR, '..', 'run', 'font'), exist_ok=True)
font_dst = os.path.join(UPSTREAM, 'font')
os.makedirs(font_dst, exist_ok=True)
if os.path.isdir(FONTS):
    for fn in os.listdir(FONTS):
        if fn.lower().endswith(('.ttf', '.ttc')):
            shutil.copy2(os.path.join(FONTS, fn), os.path.join(font_dst, fn))
            print(f'    字体 {fn}')

print('\n补丁注入完成 ✅')
