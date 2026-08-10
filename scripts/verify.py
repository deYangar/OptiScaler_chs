# -*- coding: utf-8 -*-
"""验证：localization.h enum 数 == lang_en.h == lang_zh_cn.h 条目数"""
import io, re, sys

repo = r'C:\Users\Yang\.openclaw\workspace\projects\optiscaler-chs-repo'
loc = io.open(repo + r'\overlay\OptiScaler\localization\localization.h', encoding='utf-8').read()
en = io.open(repo + r'\overlay\OptiScaler\localization\lang_en.h', encoding='utf-8').read()
zh = io.open(repo + r'\overlay\OptiScaler\localization\lang_zh_cn.h', encoding='utf-8').read()

# enum 成员（排除 COUNT 和 Language 枚举）
enum_names = set(re.findall(r'^\s{4}([A-Za-z_][A-Za-z0-9_]*),', loc, re.M))
enum_names.discard('COUNT')
# Language enum 里的成员（English/ChineseSimplified 等）
lang_members = {'English', 'ChineseSimplified'}

en_keys = set(re.findall(r'LK::(\w+)', en))
zh_keys = set(re.findall(r'LK::(\w+)', zh))

print(f'enum 成员数（含 Language）: {len(enum_names)}')
print(f'lang_en 条目: {len(en_keys)}')
print(f'lang_zh 条目: {len(zh_keys)}')

# enum 里有但翻译表没有的
missing_en = enum_names - en_keys - {'COUNT'}
missing_zh = enum_names - zh_keys - {'COUNT'}
extra_en = en_keys - enum_names
extra_zh = zh_keys - enum_names

print(f'\nenum 存在但 lang_en 缺失: {sorted(missing_en)}')
print(f'enum 存在但 lang_zh 缺失: {sorted(missing_zh)}')
print(f'lang_en 有但 enum 没有: {sorted(extra_en)}')
print(f'lang_zh 有但 enum 没有: {sorted(extra_zh)}')

# menu_common.cpp 使用的 key 是否都在 enum
mc = io.open(repo + r'\OptiScaler\menu\menu_common.cpp', encoding='utf-8').read()
used = set(re.findall(r'LOC\((\w+)\)', mc))
missing_used = used - enum_names
print(f'\nmenu_common.cpp 使用 {len(used)} 个 LOC key，缺失: {sorted(missing_used)[:20]}')
