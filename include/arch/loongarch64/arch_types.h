/**
 * @file    arch_types.h
 * @brief   LoongArch64架构类型定义
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件定义LoongArch64架构相关类型和工具函数
 *          - 中断标志类型
 *          - 字节反转查找表
 *          - 64位除法辅助函数
 *
 * @note MISRA-C:2012合规
 * @note 提供64位除法的软件实现
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef _ARCH_TYPES_H
#define _ARCH_TYPES_H

/* ==================== 头文件包含 ==================== */
#include <system/types.h>

/* ==================== 宏定义 ==================== */
/* 本文件不定义宏 */

/* ==================== 类型定义 ==================== */

/**
 * @brief 中断标志类型
 *
 * @details 用于保存中断状态
 */
typedef unsigned long irq_flags_t;

/* ==================== 内联函数 ==================== */

/**
 * @defgroup BitOperations 位操作函数
 * @{
 */

/**
 * @brief 字节反转查找表
 *
 * @details 用于位反转操作的查找表
 *          将8位值的位顺序反转
 */
static const unsigned char byte_reverse_table[] = {
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0, 0x08, 0x88, 0x48,
    0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8, 0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4,
    0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4, 0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C,
    0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC, 0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2,
    0x32, 0xB2, 0x72, 0xF2, 0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A,
    0xFA, 0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6, 0x0E, 0x8E,
    0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE, 0x01, 0x81, 0x41, 0xC1, 0x21,
    0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1, 0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
    0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9, 0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55,
    0xD5, 0x35, 0xB5, 0x75, 0xF5, 0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD,
    0x7D, 0xFD, 0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3, 0x0B,
    0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB, 0x07, 0x87, 0x47, 0xC7,
    0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7, 0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F,
    0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF};

/**
 * @brief 查找最高置位位（64位）
 *
 * @details 计算64位值中最高置位位的位置
 *
 * @param value 要查找的64位值
 *
 * @return 返回最高置位位的位数
 */
static inline u32 do_fls64(u64 value)
{
    u32 num_bits = 0U;

    if (value & 0xFFFF000000000000ULL)
    {
        num_bits += 16U;
        value = value >> 16U;
    }
    if (value & 0x0000FFFF00000000ULL)
    {
        num_bits += 16U;
        value = value >> 16U;
    }
    if (value & 0x00000000FFFF0000ULL)
    {
        num_bits += 16U;
        value = value >> 16U;
    }
    if (value & 0x000000000000FF00ULL)
    {
        num_bits += 8U;
        value = value >> 8U;
    }
    if (value & 0x00000000000000F0ULL)
    {
        num_bits += 4U;
        value = value >> 4U;
    }
    if (value & 0x000000000000000CULL)
    {
        num_bits += 2U;
        value = value >> 2U;
    }
    if (value & 0x0000000000000003ULL)
    {
        num_bits += 2U;
        value = value >> 2U;
    }
    return num_bits;
}

/** @} */

/**
 * @defgroup DivisionHelpers 除法辅助函数
 * @{
 */

/**
 * @brief 64位无符号除法（软件实现）
 *
 * @details 实现64位无符号整数除法
 *          返回商和余数
 *
 * @param dividend  被除数
 * @param divisor   除数
 * @param remainder 余数指针（可为NULL）
 *
 * @return 返回商
 *
 * @note 如果除数为0，函数会进入死循环
 */
static inline u64 do_udiv64(u64 dividend, u64 divisor, u64 *remainder)
{
    u32 num_bits;
    register u8 *p;
    register u8 *q;
    u64 quotient;
    u64 remaind;

    if (divisor == 0U)
    {
        for (;;)
        { /* MISRA compliance: 死循环表示除零错误 */
        }
    }

    if (dividend <= divisor)
    {
        remaind = divisor - dividend;
        if (remainder != NULL)
        {
            *remainder = (remaind != 0U) ? dividend : 0U;
        }
        return (remaind != 0U) ? 0U : 1U;
    }

    remaind = 0U;
    num_bits = do_fls64(dividend);
    dividend = dividend << (64U - num_bits);
    p = (u8 *)&dividend;
    q = (u8 *)&quotient;
    q[7] = byte_reverse_table[p[0]];
    q[6] = byte_reverse_table[p[1]];
    q[5] = byte_reverse_table[p[2]];
    q[4] = byte_reverse_table[p[3]];
    q[3] = byte_reverse_table[p[4]];
    q[2] = byte_reverse_table[p[5]];
    q[1] = byte_reverse_table[p[6]];
    q[0] = byte_reverse_table[p[7]];
    dividend = quotient;
    quotient = 0U;

    /* 除法循环（TODO: 应使用架构特定的汇编代码优化） */
    while (num_bits != 0U)
    {
        remaind = (remaind << 1U) | (dividend & 0x1U);
        dividend = dividend >> 1U;
        if (remaind < divisor)
        {
            quotient = (quotient << 1U) | 0U;
        }
        else
        {
            quotient = (quotient << 1U) | 1U;
            remaind = remaind - divisor;
        }
        num_bits--;
    }

    if (remainder != NULL)
    {
        *remainder = remaind;
    }

    return quotient;
}

/**
 * @brief 64位无符号除法
 *
 * @details 执行64位无符号除法，仅返回商
 *
 * @param value   被除数
 * @param divisor 除数
 *
 * @return 返回商
 */
static inline u64 arch_udiv64(u64 value, u64 divisor)
{
    return do_udiv64(value, divisor, NULL);
}

/** @} */

/* ==================== C++兼容性 ==================== */
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif /* _ARCH_TYPES_H */
