#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
translate_setup_bat.py - setup_windows.bat 增量汉化同步工具

模式与 sync_docs.py 完全一致：
1. 从 upstream remote 读取最新 setup_windows.bat（英文原版）
2. 与基线快照 docs/upstream_ref/setup_windows.bat 对比
3. 无变化 -> exit 0（不产生任何改动）
4. 有变化 -> 应用汉化映射表，生成 GBK+CRLF 的 setup_windows.bat，更新基线 -> exit 2（需提交）

汉化方式（确定性，无需 LLM）：
- 逐行匹配映射表（setup_bat_mapping.MAPPING）：英文显示文本行 -> 中文行
- 锚点行（EXTRA）：在指定位置插入汉化版额外内容（说明横幅等）
- HEADER：文件开头插入汉化版注释块
- 未命中的行保留英文原样，并输出警告列表（上游改动新文本时提醒补充映射）

用法：
  python scripts/translate_setup_bat.py --repo <仓库根> [--dry-run]

退出码：0 = 无变更；2 = 有变更（需提交）；其他 = 异常
"""
import argparse
import io
import os
import subprocess
import sys

try:
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    sys.stderr.reconfigure(encoding='utf-8', errors='replace')
except Exception:
    pass

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from setup_bat_mapping import MAPPING, EXTRA, HEADER

UPSTREAM_REMOTE = 'upstream'
UPSTREAM_BRANCH = 'master'
REF_PATH = 'docs/upstream_ref/setup_windows.bat'
TARGET_PATH = 'setup_windows.bat'


def run_git(repo, args, check=True):
    p = subprocess.run(['git', '-C', repo] + args, capture_output=True, text=True, encoding='utf-8', errors='replace')
    if check and p.returncode != 0:
        raise RuntimeError(f'git {" ".join(args)} 失败: {p.stderr[:500]}')
    return p.stdout


def read_file(path):
    if not os.path.exists(path):
        return None
    with io.open(path, encoding='utf-8-sig', errors='replace') as f:
        return f.read()


def write_utf8(path, content):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with io.open(path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(content)


def _is_noise(line):
    """判断是否为无需翻译的行（ASCII art / 选项列表 / 卸载脚本生成代码）"""
    import re
    # ASCII art / 纯符号行（无英文字母）
    if re.match(r'echo\s+[^a-zA-Z]+$', line):
        return True
    # 选项行 [1] [2]...
    if re.match(r'echo\s+\[\d\]', line):
        return True
    # 卸载脚本生成代码行
    if re.match(r'echo\s+(echo|set|if |for |del |rd |pause|cls|@echo|REM |\(|\)|\^)', line):
        return True
    if re.match(r'echo\s+echo\.', line):
        return True
    return False


def translate(upstream_lines):
    """应用映射表，返回 (汉化行列表, 未命中行列表)"""
    out = []
    unmatched = []
    for line in upstream_lines:
        rline = line.rstrip('\r\n')
        key = rline
        if key in MAPPING:
            out.append(MAPPING[key])
        else:
            out.append(rline)
            # 记录未命中（只关心真正的英文显示文本，排除：ASCII art / 选项行 / 卸载脚本生成行）
            stripped = rline.strip()
            is_display = stripped.startswith('echo ') and not stripped.startswith(('echo.', 'echo:'))
            is_prompt = 'set /p ' in stripped
            if (is_display or is_prompt) and not _is_noise(stripped):
                unmatched.append(rline)
        if key in EXTRA:
            out.extend(EXTRA[key])
    return HEADER + out, unmatched


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--repo', required=True)
    ap.add_argument('--dry-run', action='store_true')
    ap.add_argument('--force', action='store_true', help='忽略基线对比，强制重新汉化')
    args = ap.parse_args()

    repo = os.path.abspath(args.repo)

    # 1. 读取上游最新版
    upstream = run_git(repo, ['show', f'{UPSTREAM_REMOTE}/{UPSTREAM_BRANCH}:setup_windows.bat'])
    if upstream is None:
        print('⚠️ 无法读取上游 setup_windows.bat，跳过')
        return 1

    # 2. 对比基线
    baseline = read_file(os.path.join(repo, REF_PATH))
    if not args.force and baseline is not None and baseline == upstream:
        print('ℹ️ 上游 setup_windows.bat 无变化，跳过汉化')
        return 0

    print('🔁 检测到上游 setup_windows.bat 有更新，开始汉化...')

    # 3. 应用映射
    chs_lines, unmatched = translate(upstream.splitlines())
    content = '\r\n'.join(chs_lines) + '\r\n'

    if args.dry_run:
        print(f'[dry-run] 将写入 setup_windows.bat（GBK, {len(content.encode("gbk", errors="replace"))} 字节）')
    else:
        with io.open(os.path.join(repo, TARGET_PATH), 'wb') as f:
            f.write(content.encode('gbk', errors='replace'))
        write_utf8(os.path.join(repo, REF_PATH), upstream)
        print(f'✅ 已生成 setup_windows.bat（GBK+CRLF，{len(chs_lines)} 行）')
        print(f'✅ 已更新基线 {REF_PATH}')

    # 4. 未命中报告
    if unmatched:
        print(f'\n⚠️ {len(unmatched)} 行未命中映射（保留英文，请补充翻译）：')
        for u in unmatched:
            print('   ', repr(u))

    return 2


if __name__ == '__main__':
    sys.exit(main())
