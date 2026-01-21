#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
代码风格重构脚本
用于批量修改 LoongArch64 目录下所有 .c 文件的代码风格
"""

import os
import re
import sys

# 需要处理的文件列表
FILES = [
    "arch_run2user.c",
    "arch_tls.c",
    "atomic.c",
    "cache.c",
    "context_cpu_state.c",
    "cpu.c",
    "mmu.c",
    "ptrace.c",
    "exception.c",
    "arch.c",
    "arch_signal.c",
    "arch_uaccess.c",
    "cpu_info.c",
    "asm-offsets.c",
    "coredump.c",
    "generic_timer.c",
    "ipi.c",
    "loongarch_timer.c",
    "tlb.c",
    "ttosConfig.c",
    "arch_init.c",
    "backtrace.c"
]

def fix_comment_separators(content):
    """修复注释分隔符格式"""
    # 将旧的注释分隔符格式转换为新的格式
    # 例如：/************************头 文 件******************************/
    # 转换为：/*************************** 头文件包含 ****************************/

    # 定义替换映射
    replacements = [
        (r'/\*.*头 文 件.*\*/', '/*************************** 头文件包含 ****************************/'),
        (r'/\*.*外部函数声明.*\*/', '/*************************** 外部函数声明 ****************************/'),
        (r'/\*.*外部声明.*\*/', '/*************************** 外部声明 ****************************/'),
        (r'/\*.*宏 定 义.*\*/', '/*************************** 宏定义 ****************************/'),
        (r'/\*.*类型定义.*\*/', '/*************************** 类型定义 ****************************/'),
        (r'/\*.*全局变量.*\*/', '/*************************** 全局变量 ****************************/'),
        (r'/\*.*模块变量.*\*/', '/*************************** 模块变量 ****************************/'),
        (r'/\*.*前向声明.*\*/', '/*************************** 前向声明 ****************************/'),
        (r'/\*.*函数实现.*\*/', '/*************************** 函数实现 ****************************/'),
    ]

    for pattern, replacement in replacements:
        content = re.sub(pattern, replacement, content)

    return content

def fix_function_spacing(content):
    """修复函数之间的空行"""
    # 在函数的 } 和下一个函数的 /** 之间添加空行
    # 这种模式：}    /**  -> }\n\n    /**
    lines = content.split('\n')
    result = []
    i = 0

    while i < len(lines):
        result.append(lines[i])

        # 如果当前行是函数结束的 }
        if lines[i].strip() == '}':
            # 检查下一行是否是 /** 或空白
            if i + 1 < len(lines):
                next_line = lines[i + 1].strip()
                # 如果下一行是 /** 且中间没有空行
                if next_line.startswith('/**'):
                    # 检查中间是否有空行
                    has_empty = False
                    j = i + 1
                    while j < len(lines) and lines[j].strip() == '':
                        has_empty = True
                        j += 1

                    if not has_empty:
                        result.append('')  # 添加空行

        i += 1

    return '\n'.join(result)

def fix_return_spacing(content):
    """修复 return 语句前的空行"""
    lines = content.split('\n')
    result = []
    i = 0

    while i < len(lines):
        result.append(lines[i])

        # 如果当前行是 return 语句
        stripped = lines[i].strip()
        if re.match(r'^return\s+.*;', stripped) or stripped == 'return;':
            # 检查上一行是否是空行
            if i > 0 and lines[i - 1].strip() != '':
                # 检查是否在函数内部（不是最后一行）
                in_function = False
                for j in range(i - 1, max(0, i - 5), -1):
                    if lines[j].strip() == '}':
                        break
                    if lines[j].strip() == '{':
                        in_function = True
                        break

                # 检查 return 后面是否是 }
                is_last_statement = False
                if i + 1 < len(lines):
                    next_stripped = lines[i + 1].strip()
                    if next_stripped == '}':
                        is_last_statement = True

                # 如果是函数内的 return 且不是最后一条语句
                if in_function and not is_last_statement:
                    # 检查上一行是否已经是空行
                    if i > 0 and lines[i - 1].strip() != '':
                        result.pop()  # 移除刚添加的行
                        result.append('')  # 添加空行
                        result.append(lines[i])  # 重新添加当前行

        i += 1

    return '\n'.join(result)

def fix_pointer_declaration(content):
    """修复指针声明空格格式：type *ptr"""
    # 这个比较复杂，需要仔细处理
    # 简单的处理：将 type* ptr 或 type *ptr 转换为 type *ptr
    # 但要小心不要破坏其他内容

    # 这个函数留作后续实现，因为需要非常小心处理
    return content

def process_file(filepath):
    """处理单个文件"""
    print(f"处理文件: {filepath}")

    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()

        # 应用所有修复
        content = fix_comment_separators(content)
        content = fix_function_spacing(content)
        content = fix_return_spacing(content)

        # 写回文件
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)

        print(f"  ✓ 完成")
        return True
    except Exception as e:
        print(f"  ✗ 错误: {e}")
        return False

def main():
    """主函数"""
    # 获取脚本所在目录
    script_dir = os.path.dirname(os.path.abspath(__file__))

    success_count = 0
    fail_count = 0

    for filename in FILES:
        filepath = os.path.join(script_dir, filename)

        if os.path.exists(filepath):
            if process_file(filepath):
                success_count += 1
            else:
                fail_count += 1
        else:
            print(f"文件不存在: {filepath}")
            fail_count += 1

    print(f"\n处理完成:")
    print(f"  成功: {success_count}")
    print(f"  失败: {fail_count}")

    return 0 if fail_count == 0 else 1

if __name__ == '__main__':
    sys.exit(main())
