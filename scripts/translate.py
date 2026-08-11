#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
自动翻译新字符串（DeepSeek API）并更新汉化文件
用法: python translate.py --input pending.json [--key <API_KEY>]
流程:
  1. 读待翻译列表
  2. 调 DeepSeek 翻译（带术语表保证一致性）
  3. 生成新 LK key → 更新 localization.h / lang_en.h / lang_zh_cn.h / strings_map.json
"""
import io
import json
import os
import re
import sys
import urllib.request

HERE = os.path.dirname(os.path.abspath(__file__))
LOC_H = os.path.normpath(os.path.join(HERE, '..', 'overlay', 'OptiScaler', 'localization', 'localization.h'))
EN_H = os.path.normpath(os.path.join(HERE, '..', 'overlay', 'OptiScaler', 'localization', 'lang_en.h'))
ZH_H = os.path.normpath(os.path.join(HERE, '..', 'overlay', 'OptiScaler', 'localization', 'lang_zh_cn.h'))
MAP = os.path.join(HERE, 'strings_map.json')

# 术语表：保证翻译一致性
GLOSSARY = """游戏/计算机术语对照表（必须严格遵守）：
- Upscaler = 超分辨率
- Upscaling = 超分
- Frame Generation / FG = 帧生成
- Motion Vectors / MV = 运动矢量
- HUDLess = HUDLess（保留）
- Reactive Mask = Reactive Mask（保留）
- Ghosting = 拖影
- Artifacts = 伪影
- Overlay = 覆盖层
- V-Sync = 垂直同步
- Shader = 着色器
- Preset = 预设
- Sharpness = 锐化
- Depth = 深度
- Latency = 延迟
- Toggle = 切换
- Hook = 挂钩
- Tooltip = 提示
- DLSS/FSR/XeSS/RCAS/DA/MAS/MFG/DLSSG/XeFG/OptiFG = 保留英文
- PRESSET A~O / DEFAULT / Latest = 保留英文
- Don't / Can't = 不要 / 不能
"""

def call_deepseek(api_key, texts):
    """批量翻译"""
    prompt = GLOSSARY + '\n请把以下 JSON 数组中的英文 UI 字符串翻译成简体中文（游戏软件风格，保留 %s/%d/%llu 等格式符和 \\n 换行，保留 HTML/技术术语），输出 JSON 数组，一一对应：\n' + json.dumps(texts, ensure_ascii=False)
    body = json.dumps({
        "model": "deepseek-chat",
        "messages": [{"role": "user", "content": prompt}],
        "temperature": 0.3,
        "max_tokens": 4096,
    }).encode('utf-8')
    req = urllib.request.Request(
        'https://api.deepseek.com/chat/completions',
        data=body,
        headers={'Content-Type': 'application/json', 'Authorization': f'Bearer {api_key}'},
    )
    with urllib.request.urlopen(req, timeout=120) as resp:
        data = json.loads(resp.read().decode('utf-8'))
    content = data['choices'][0]['message']['content']
    # 提取 JSON 数组
    m = re.search(r'\[.*\]', content, re.S)
    if not m:
        raise RuntimeError(f'无法解析翻译结果: {content[:300]}')
    return json.loads(m.group(0))


def call_deepseek_retry(api_key, texts, max_retries=3):
    """调用 DeepSeek 翻译，返回与 texts 等长的列表；失败/丢条时重试，最终用原文兜底"""
    res = None
    last_err = None
    for attempt in range(1, max_retries + 1):
        try:
            res = call_deepseek(api_key, texts)
            if not isinstance(res, list):
                raise RuntimeError(f'返回不是数组: {type(res)}')
            if len(res) == len(texts):
                return res
            last_err = f'数量不匹配: {len(res)} != {len(texts)}'
            print(f'    重试 {attempt}: {last_err}')
        except Exception as e:
            last_err = str(e)
            print(f'    重试 {attempt}: {e}')
    # 重试耗尽：按位置尽量对齐，缺失的用原文兜底
    out = []
    for i, t in enumerate(texts):
        if res is not None and i < len(res) and isinstance(res[i], str) and res[i].strip():
            out.append(res[i])
        else:
            out.append(t)
    fallback_cnt = sum(1 for i, t in enumerate(texts)
                       if not (res is not None and i < len(res) and isinstance(res[i], str) and res[i].strip()))
    print(f'    ⚠️ 重试耗尽（{last_err}），{fallback_cnt} 条用原文兜底')
    return out

def key_from_en(en):
    """英文串 → 合法 key 名"""
    words = re.findall(r'[A-Za-z0-9]+', en)
    name = '_'.join(words)[:60] or 'Str'
    if not name[0].isalpha():
        name = 'S_' + name
    return f'UI_{name}'

def main():
    args = sys.argv[1:]
    inp = None
    api_key = os.environ.get('DEEPSEEK_API_KEY', '')
    if '--input' in args:
        inp = args[args.index('--input') + 1]
    if '--key' in args:
        api_key = args[args.index('--key') + 1]
    if not inp or not api_key:
        print('用法: translate.py --input pending.json --key <API_KEY>')
        sys.exit(1)

    with io.open(inp, encoding='utf-8') as f:
        pending = json.load(f)
    if not pending:
        print('没有待翻译字符串')
        return

    # 分批（每批 50 条），带重试+原文兜底
    zh_all = []
    BATCH = 50
    for i in range(0, len(pending), BATCH):
        batch = pending[i:i + BATCH]
        print(f'翻译批次 {i // BATCH + 1}/{(len(pending) + BATCH - 1) // BATCH} ({len(batch)} 条)...')
        zh_all.extend(call_deepseek_retry(api_key, batch))

    if len(zh_all) != len(pending):
        print(f'!! 翻译数量不匹配: {len(zh_all)} != {len(pending)}')
        sys.exit(1)

    # 生成 key（避免与现有冲突）
    with io.open(LOC_H, encoding='utf-8') as f:
        loc_h = f.read()
    existing = set(re.findall(r'^\s{4}([A-Za-z_][A-Za-z0-9_]*),', loc_h, re.M))

    new_entries = []  # (key, en, zh)
    for en, zh in zip(pending, zh_all):
        key = key_from_en(en)
        base = key
        i = 2
        while key in existing:
            key = f'{base}_{i}'
            i += 1
        existing.add(key)
        new_entries.append((key, en, zh))

    # 更新 localization.h
    with io.open(LOC_H, encoding='utf-8') as f:
        loc_h = f.read()
    members = '\n'.join(f'    {k},' for k, _, _ in new_entries)
    loc_h = loc_h.replace('    COUNT\n};', members + '\n    COUNT\n};')
    with io.open(LOC_H, 'w', encoding='utf-8', newline='\n') as f:
        f.write(loc_h)

    # 更新 lang_en.h / lang_zh_cn.h
    for path, is_zh in ((EN_H, False), (ZH_H, True)):
        with io.open(path, encoding='utf-8') as f:
            c = f.read()
        lines = []
        for k, en, zh in new_entries:
            v = zh if is_zh else en
            lines.append(f'    table[static_cast<int>(LK::{k})] = "{v}";')
        c = c.rstrip() + '\n' + '\n'.join(lines) + '\n}\n'
        with io.open(path, 'w', encoding='utf-8', newline='\n') as f:
            f.write(c)

    # 更新 strings_map.json
    with io.open(MAP, encoding='utf-8') as f:
        smap = json.load(f)
    for k, en, _ in new_entries:
        smap[en] = k
    with io.open(MAP, 'w', encoding='utf-8', newline='\n') as f:
        json.dump(smap, f, ensure_ascii=False, indent=1)

    print(f'✅ 新增 {len(new_entries)} 条翻译')
    for k, en, zh in new_entries[:10]:
        print(f'  {k}: {en[:40]} -> {zh[:40]}')
    if len(new_entries) > 10:
        print(f'  ... 其余 {len(new_entries) - 10} 条')

if __name__ == '__main__':
    main()
