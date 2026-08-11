#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
扫描上游源码中未汉化的 UI 英文字符串
用法: python scan_strings.py <上游源码根目录> [--output pending.json]
输出: 不在 strings_map.json 中的英文 UI 串列表（用户可见的）
"""
import io
import json
import os
import re
import sys
# Windows 控制台默认 cp1252，强制 UTF-8 输出避免 UnicodeEncodeError
if hasattr(sys.stdout, 'reconfigure'):
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    sys.stderr.reconfigure(encoding='utf-8', errors='replace')
else:
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')


HERE = os.path.dirname(os.path.abspath(__file__))

def load_strings_map():
    with io.open(os.path.join(HERE, 'strings_map.json'), encoding='utf-8') as f:
        return json.load(f)

def scan_file(path, strings_map):
    """返回 (新增串列表, 文件里已翻译数)"""
    with io.open(path, encoding='utf-8', errors='replace') as f:
        lines = f.readlines()

    pending = []
    translated = 0
    pat = re.compile(r'"((?:[^"\\]|\\.)*)"')
    skip_prefixes = ('//', '*', 'LOG_', 'IM_ASSERT', 'IMGUI_DEBUG', 'std::', 'return ', 'case ')
    for line in lines:
        s = line.lstrip()
        if s.startswith(skip_prefixes):
            continue
        if 'LOC(' in line:
            translated += 1
            continue
        for m in pat.finditer(line):
            raw = m.group(1)
            if len(raw) < 5:
                continue
            # 纯符号/格式串
            if re.fullmatch(r'[%#0-9.\s\-+*/\\|,;:()\[\]{}<>!?~^&@$=\'"_]+', raw):
                continue
            # 含中文 → 已翻译
            if re.search(r'[\u4e00-\u9fff]', raw):
                continue
            # 单个标识符/单词（无空格、无格式符）→ 跳过（技术名/变量）
            if ' ' not in raw and '%' not in raw and re.fullmatch(r'[A-Za-z0-9_.-]+', raw):
                continue
            # 文件路径/include 文件名（含 / 或 \ 的路径，或 .h/.cpp 等文件后缀）→ 跳过
            if re.fullmatch(r'[A-Za-z0-9_./\\\-]+', raw) and ('.h' in raw or '.cpp' in raw or '.c' in raw or '.ttf' in raw or '.ttc' in raw or '.ini' in raw or '.json' in raw or '.png' in raw or '/' in raw or '\\' in raw):
                continue
            # 已翻译过的串
            if raw in strings_map:
                continue
            pending.append(raw)
    return pending, translated

def main():
    upstream = os.path.abspath(sys.argv[1])
    out = sys.argv[2] if len(sys.argv) > 2 and sys.argv[2].startswith('--output') else None
    if out:
        out = sys.argv[sys.argv.index('--output') + 1]
    strings_map = load_strings_map()

    targets = [
        os.path.join(upstream, 'OptiScaler', 'menu', 'menu_common.cpp'),
    ]
    all_pending = []
    for t in targets:
        if os.path.exists(t):
            pending, translated = scan_file(t, strings_map)
            print(f'{os.path.basename(t)}: 已翻译调用 {translated}，新增未翻译 {len(pending)}')
            all_pending.extend(pending)

    # 去重保序
    seen = set()
    uniq = [x for x in all_pending if not (x in seen or seen.add(x))]
    print(f'\n共发现 {len(uniq)} 条新字符串')

    if out:
        with io.open(out, 'w', encoding='utf-8') as f:
            json.dump(uniq, f, ensure_ascii=False, indent=1)
        print(f'已写入 {out}')

if __name__ == '__main__':
    main()
