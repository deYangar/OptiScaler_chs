#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
sync_docs.py - 汉化文档增量同步工具

背景：workflow 同步上游时排除了已汉化文档（README/Config/Features/Issues/Spoofing/
CONTRIBUTING/OptiScaler.ini），防止上游覆盖中文版。本脚本负责"排除后的跟随更新"：

1. OptiScaler.ini（结构化，全自动合并）：
   - 对比上游英文版与本地中文版的 section/key 结构
   - 上游新增 key -> 用 strings_map.json 已有翻译，没有的调 DeepSeek 翻译注释，插入中文版
   - 上游删除 key -> 从中文版删除
   - key 顺序变化 -> 按上游顺序重排（本地已翻译的注释保留不动）
2. md 文档（README/Config/Features/Issues/Spoofing/CONTRIBUTING，半自动）：
   - 检测上游是否有变化
   - 有变化 -> 调 DeepSeek 翻译新增内容，更新中文版（尽力而为）
   - 无法自动翻译的部分 -> 写 docs/UPSTREAM_CHANGES.md 待翻译清单

用法：
  python scripts/sync_docs.py --repo <仓库根> [--api-key <DeepSeek key>] [--dry-run]

依赖：Python 3.8+，标准库（urllib/json/re），无第三方依赖。
"""
import argparse
import io
import json
import os
import re
import subprocess
import sys
import urllib.request

try:
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    sys.stderr.reconfigure(encoding='utf-8', errors='replace')
except Exception:
    pass

# 需要跟随上游的文档清单（已汉化、从 sync checkout 排除的文件）
MD_DOCS = ['README.md', 'Config.md', 'Features.md', 'Issues.md', 'Spoofing.md', 'CONTRIBUTING.md']
INI_DOCS = ['OptiScaler.ini']
UPSTREAM_REMOTE = 'upstream'
UPSTREAM_BRANCH = 'master'

GLOSSARY = """
术语表（必须遵守）：
- DLSS/FSR/XeSS/RCAS/MAS/MFG/DLSSG/XeFG/OptiFG/FG/HUD/VRAM = 保留英文
- Upscaler = 超分辨率技术（超分）
- Frame Generation = 帧生成
- Default (auto) = 默认 (auto)
- true or false = true 或 false
- DLL 文件名/路径/URL/十六进制值 = 保留原样
- 游戏名（如 Cyberpunk 2077） = 保留英文
"""


def run_git(repo, args, check=True):
    """在 repo 中执行 git 命令，返回 stdout 文本"""
    p = subprocess.run(['git', '-C', repo] + args, capture_output=True, text=True, encoding='utf-8', errors='replace')
    if check and p.returncode != 0:
        raise RuntimeError(f'git {" ".join(args)} 失败: {p.stderr[:500]}')
    return p.stdout


def read_file(path):
    if not os.path.exists(path):
        return None
    with io.open(path, encoding='utf-8-sig', errors='replace') as f:
        return f.read()


def write_file(path, content):
    with io.open(path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(content)


def get_upstream_file(repo, path):
    """从 upstream remote 读取文件内容（不覆盖工作区）"""
    try:
        return run_git(repo, ['show', f'{UPSTREAM_REMOTE}/{UPSTREAM_BRANCH}:{path}'])
    except RuntimeError:
        return None


def call_deepseek(api_key, texts):
    """批量翻译注释文本（DeepSeek），返回与 texts 等长的列表"""
    if not api_key:
        return [t for t in texts]  # 无 key 时原文兜底
    prompt = GLOSSARY + '\n请把以下 JSON 数组中的英文配置注释翻译成简体中文（保持配置文件注释风格，保留 %s/%d 等格式符、数值、DLL 文件名、URL、十六进制值、True/False 选项值不翻译），输出 JSON 数组，一一对应：\n' + json.dumps(texts, ensure_ascii=False)
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
    with urllib.request.urlopen(req, timeout=180) as resp:
        data = json.loads(resp.read().decode('utf-8'))
    content = data['choices'][0]['message']['content']
    m = re.search(r'\[.*\]', content, re.S)
    if not m:
        raise RuntimeError(f'无法解析翻译结果: {content[:300]}')
    return json.loads(m.group(0))


def call_deepseek_retry(api_key, texts, max_retries=3):
    """重试版翻译，失败用原文兜底"""
    res = None
    last_err = None
    for attempt in range(1, max_retries + 1):
        try:
            res = call_deepseek(api_key, texts)
            if isinstance(res, list) and len(res) == len(texts):
                return res
            last_err = f'数量不匹配: {len(res) if isinstance(res, list) else type(res)} != {len(texts)}'
            print(f'  重试 {attempt}: {last_err}')
        except Exception as e:
            last_err = str(e)
            print(f'  重试 {attempt}: {e}')
    out = []
    for i, t in enumerate(texts):
        if res is not None and i < len(res) and isinstance(res[i], str) and res[i].strip():
            out.append(res[i])
        else:
            out.append(t)
    return out


def load_strings_map(repo):
    """读取 strings_map.json，返回 {英文: 中文} 映射"""
    path = os.path.join(repo, 'scripts', 'strings_map.json')
    try:
        with io.open(path, encoding='utf-8') as f:
            data = json.load(f)
        if isinstance(data, dict):
            return data
    except Exception:
        pass
    return {}


def parse_ini(text):
    """解析 ini 为结构化条目列表。
    返回 [{type: 'section', name}, {type: 'entry', key, comments: [..], value_line}]
    以及 {key: 注释文本} 用于翻译。
    """
    entries = []
    cur_comments = []
    for line in text.splitlines():
        stripped = line.strip()
        if stripped.startswith('[') and stripped.endswith(']'):
            if cur_comments:
                # 悬空注释（section 前的分隔线等）归入下一个 entry 的注释
                pass
            entries.append({'type': 'section', 'name': stripped})
            cur_comments = []
        elif stripped.startswith(';'):
            cur_comments.append(line)
        elif '=' in stripped and not stripped.startswith(';'):
            key = stripped.split('=', 1)[0].strip()
            entries.append({
                'type': 'entry',
                'key': key,
                'comments': cur_comments,
                'value_line': line,
            })
            cur_comments = []
        else:
            # 空行或其他
            if cur_comments:
                # 注释后遇到空行，注释保留给下一个 entry
                pass
            entries.append({'type': 'blank'})
    return entries


def merge_ini(upstream_text, local_text, strings_map, api_key):
    """增量合并：以本地中文版为基准，跟随上游结构。
    返回 (新内容, 变更统计 dict)
    """
    up = parse_ini(upstream_text)
    local = parse_ini(local_text)

    # 本地 key -> entry 映射（保留本地翻译）
    local_by_key = {}
    for e in local:
        if e['type'] == 'entry':
            local_by_key[e['key']] = e

    # 上游 key 顺序
    up_keys = [e['key'] for e in up if e['type'] == 'entry']

    # 需要翻译的：上游有、本地没有的 entry 注释
    to_translate = []
    new_entries = []
    for e in up:
        if e['type'] == 'entry' and e['key'] not in local_by_key:
            to_translate.append(e)

    # 批量翻译新增注释
    if to_translate:
        texts = ['\n'.join(e['comments']) if e['comments'] else '' for e in to_translate]
        # 优先用 strings_map 查整段？不适用，按 key 查注释
        translated = call_deepseek_retry(api_key, texts)
        for e, t in zip(to_translate, translated):
            e['_translated_comments'] = t.splitlines() if t else []

    # 重建文件
    out_lines = []
    stats = {'added': 0, 'removed': 0, 'kept': 0, 'reordered': 0}

    # 本地有、上游没有的 key（将被移除）
    local_keys = set(local_by_key.keys())
    up_key_set = set(up_keys)
    removed = local_keys - up_key_set
    stats['removed'] = len(removed)

    # 顺序跟随上游；section 分割
    prev_type = None
    for e in up:
        if e['type'] == 'section':
            if prev_type == 'entry':
                out_lines.append('')
            out_lines.append(e['name'])
            prev_type = 'section'
        elif e['type'] == 'entry':
            if e['key'] in local_by_key:
                le = local_by_key[e['key']]
                if le['comments']:
                    out_lines.extend(le['comments'])
                else:
                    out_lines.extend(['; ' + c for c in e['comments']])
                # 值行：本地保留（可能有用户修改），否则用上游
                out_lines.append(le['value_line'])
                stats['kept'] += 1
            else:
                if e.get('_translated_comments'):
                    out_lines.extend(e['_translated_comments'])
                else:
                    out_lines.extend(e['comments'])
                out_lines.append(e['value_line'])
                stats['added'] += 1
            prev_type = 'entry'
        # blank 忽略（重建时统一空行）

    new_content = '\n'.join(out_lines) + '\n'
    return new_content, stats


def check_md_changes(repo, api_key):
    """检测 md 文档上游变化，尽力翻译更新，返回 (changed_files, report_lines)"""
    changed = []
    report = []
    for doc in MD_DOCS:
        up_text = get_upstream_file(repo, doc)
        if up_text is None:
            continue
        local_text = read_file(os.path.join(repo, doc))
        if local_text is None:
            local_text = ''
        if up_text == local_text:
            continue
        changed.append(doc)
        # 尝试翻译整个上游文档（本地完全汉化的情况，直接整篇替换）
        if api_key:
            try:
                translated = call_deepseek_retry(api_key, [up_text])
                if translated and translated[0] != up_text and translated[0].strip():
                    # 保留本地文件头部汉化说明（如有）
                    write_file(os.path.join(repo, doc), translated[0])
                    report.append(f'- {doc}: 已自动翻译上游更新')
                    continue
            except Exception as e:
                report.append(f'- {doc}: 自动翻译失败 ({e})')
        # 无法自动翻译：记录到待翻译清单
        report.append(f'- {doc}: 上游有更新，需要人工处理（本次未自动翻译）')
    return changed, report


def main():
    parser = argparse.ArgumentParser(description='汉化文档增量同步')
    parser.add_argument('--repo', required=True, help='仓库根目录')
    parser.add_argument('--api-key', default=os.environ.get('DEEPSEEK_API_KEY', ''), help='DeepSeek API key')
    parser.add_argument('--dry-run', action='store_true', help='只检测不写文件')
    args = parser.parse_args()

    repo = os.path.abspath(args.repo)
    strings_map = load_strings_map(repo)

    # 确保 upstream remote 存在
    remotes = run_git(repo, ['remote'])
    if UPSTREAM_REMOTE not in remotes.split():
        run_git(repo, ['remote', 'add', UPSTREAM_REMOTE, 'https://github.com/optiscaler/OptiScaler.git'])
    run_git(repo, ['fetch', UPSTREAM_REMOTE, UPSTREAM_BRANCH, '--depth', '50'])

    all_changed = []
    report_lines = []

    # 1. ini 增量合并
    for doc in INI_DOCS:
        up_text = get_upstream_file(repo, doc)
        if up_text is None:
            print(f'[ini] {doc}: 上游无此文件，跳过')
            continue
        local_text = read_file(os.path.join(repo, doc))
        if local_text is None:
            local_text = ''
        if up_text == local_text:
            print(f'[ini] {doc}: 上游无变化')
            continue
        new_content, stats = merge_ini(up_text, local_text, strings_map, args.api_key)
        print(f'[ini] {doc}: added={stats["added"]} removed={stats["removed"]} kept={stats["kept"]}')
        if stats['added'] or stats['removed']:
            all_changed.append(doc)
            report_lines.append(f'- {doc}: 新增配置 {stats["added"]} 项，移除 {stats["removed"]} 项')
            if not args.dry_run:
                write_file(os.path.join(repo, doc), new_content)

    # 2. md 变更检测
    md_changed, md_report = check_md_changes(repo, args.api_key)
    all_changed.extend(md_changed)
    report_lines.extend(md_report)

    # 3. 写待翻译清单
    if report_lines and not args.dry_run:
        docs_dir = os.path.join(repo, 'docs')
        os.makedirs(docs_dir, exist_ok=True)
        ts = __import__('datetime').datetime.now().strftime('%Y-%m-%d %H:%M')
        content = f'# 上游文档变更记录\n\n> 生成时间：{ts}\n> 说明：以下变更已检测到，需要人工或后续自动处理。\n\n'
        content += '\n'.join(report_lines) + '\n'
        write_file(os.path.join(docs_dir, 'UPSTREAM_CHANGES.md'), content)

    if all_changed:
        print(f'变更文件: {", ".join(all_changed)}')
        print('需要提交')
        sys.exit(2)  # 退出码 2 表示有变更
    print('无变更')
    sys.exit(0)


if __name__ == '__main__':
    main()
