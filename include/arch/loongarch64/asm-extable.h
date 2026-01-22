/**
 * @file    asm-extable.h
 * @brief   汇编异常表宏定义
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 本文件定义汇编异常表（exception table）相关宏
 *          - 异常表记录指令与修复地址的映射关系
 *          - 用于用户空间访问异常处理
 *          - 支持错误码和零值寄存器设置
 *
 * @note MISRA-C:2012 合规
 * @note 本文件主要用于汇编代码
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef _ASM_EXTABLE_H
#define _ASM_EXTABLE_H

/************************头 文 件******************************/
#include <gpr-num.h>

/************************宏 定 义******************************/

/**
 * @defgroup ExtableUtils 异常表工具宏
 * @{
 */

/**
 * @brief 字符串化宏（第一层）
 */
#define __stringify_1(x...) #x

/**
 * @brief 字符串化宏（第二层）
 */
#define __stringify(x...) __stringify_1(x)

/**
 * @brief 文件名和行号宏
 */
#define FILE_LINE __FILE__ ":" __stringify(__LINE__)

/** @} */

/**
 * @defgroup ExtableEntries 异常表条目宏
 * @{
 */

/**
 * @brief 原始异常表条目宏
 *
 * @details 在__ex_table段中创建异常表条目
 *          - insn: 可能出错的指令地址
 *          - fixup: 异常处理跳转地址
 *          - type: 异常类型
 *          - data: 附加数据（寄存器信息等）
 */
#define __ASM_EXTABLE_RAW(insn, fixup, type, data) \
    ".pushsection    __ex_table, \"a\"\n"          \
    ".balign        4\n"                           \
    ".long        ((" insn ") - .)\n"              \
    ".long        ((" fixup ") - .)\n"             \
    ".short        (" type ")\n"                   \
    ".short        (" data ")\n"                   \
    ".popsection\n"

/**
 * @brief 标准异常表条目宏
 *
 * @details 用于用户空间访问的标准异常修复
 */
#define _ASM_EXTABLE(insn, fixup) __ASM_EXTABLE_RAW(#insn, #fixup, __stringify(EX_TYPE_FIXUP), "0")

/**
 * @brief 异常数据寄存器位偏移
 */
#define EX_DATA_REG_ERR_SHIFT 0 /**< 错误寄存器偏移 */
#define EX_DATA_REG_ERR GENMASK(4, 0) /**< 错误寄存器掩码 */
#define EX_DATA_REG_ZERO_SHIFT 5 /**< 零值寄存器偏移 */
#define EX_DATA_REG_ZERO GENMASK(9, 5) /**< 零值寄存器掩码 */

/**
 * @brief 异常数据寄存器宏
 *
 * @details 生成寄存器在异常数据中的编码
 */
#define EX_DATA_REG(reg, gpr) "((.L__gpr_num_" #gpr ") << " __stringify(EX_DATA_REG_##reg##_SHIFT) ")"

/**
 * @brief 用户空间访问异常（带错误码和零值）
 *
 * @details 处理用户空间访问异常，设置错误码和零值寄存器
 */
#define _ASM_EXTABLE_UACCESS_ERR_ZERO(insn, fixup, err, zero) \
    __DEFINE_ASM_GPR_NUMS \
    __ASM_EXTABLE_RAW(#insn, #fixup, __stringify(EX_TYPE_UACCESS_ERR_ZERO), \
                      "(" EX_DATA_REG(ERR, err) " | " EX_DATA_REG(ZERO, zero) ")")

/**
 * @brief 用户空间访问异常（仅错误码）
 *
 * @details 处理用户空间访问异常，仅设置错误码
 */
#define _ASM_EXTABLE_UACCESS_ERR(insn, fixup, err) _ASM_EXTABLE_UACCESS_ERR_ZERO(insn, fixup, err, zero)

/** @} */

/************************类型定义******************************/
/************************外部变量******************************/
/************************外部函数******************************/
/************************内联函数******************************/
/************************C++兼容性******************************/

#endif /* _ASM_EXTABLE_H */
