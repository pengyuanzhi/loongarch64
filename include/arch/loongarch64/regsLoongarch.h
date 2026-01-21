/**
 * @file    regsLoongarch.h
 * @brief   LoongArch64寄存器定义
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件定义LoongArch64架构的寄存器接口
 *          提供寄存器访问的统一接口
 *
 * @note MISRA-C:2012 合规
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef _REGSLOONGARCH_H__
#define _REGSLOONGARCH_H__

/************************头 文 件******************************/

/************************宏 定 义******************************/

/**
 * @brief LoongArch64 通用寄存器编号定义
 *
 * @details LoongArch64 有 32 个通用寄存器（r0-r31）
 *          根据 ABI 约定，部分寄存器有特殊用途
 */
#define REG_R0          0U   /* 零寄存器（硬编码为0） */
#define REG_R1          1U   /* 保留 */
#define REG_R2          2U   /* 栈指针（sp）或全局指针（gp） */
#define REG_R3          3U   /* 栈指针（sp） */
#define REG_R4          4U   /* 参数寄存器0 / 返回值寄存器0（a0） */
#define REG_R5          5U   /* 参数寄存器1 / 返回值寄存器1（a1） */
#define REG_R6          6U   /* 参数寄存器2（a2） */
#define REG_R7          7U   /* 参数寄存器3（a3） */
#define REG_R8          8U   /* 参数寄存器4（a4） */
#define REG_R9          9U   /* 参数寄存器5（a5） */
#define REG_R10         10U  /* 参数寄存器6（a6） */
#define REG_R11         11U  /* 参数寄存器7（a7） */
#define REG_R12         12U  /* 临时寄存器0（t0） */
#define REG_R13         13U  /* 临时寄存器1（t1） */
#define REG_R14         14U  /* 临时寄存器2（t2） */
#define REG_R15         15U  /* 临时寄存器3（t3） */
#define REG_R16         16U  /* 临时寄存器4（t4） */
#define REG_R17         17U  /* 临时寄存器5（t5） */
#define REG_R18         18U  /* 临时寄存器6（t6） */
#define REG_R19         19U  /* 临时寄存器7（t7） */
#define REG_R20         20U  /* 临时寄存器8（t8） */
#define REG_R21         21U  /* 保存寄存器0（s0/返回地址fp） */
#define REG_R22         22U  /* 保存寄存器1（s1） */
#define REG_R23         23U  /* 保存寄存器2（s2） */
#define REG_R24         24U  /* 保存寄存器3（s3） */
#define REG_R25         25U  /* 保存寄存器4（s4） */
#define REG_R26         26U  /* 保存寄存器5（s5） */
#define REG_R27         27U  /* 保存寄存器6（s6） */
#define REG_R28         28U  /* 保存寄存器7（s7） */
#define REG_R29         29U  /* 保存寄存器8（s8） */
#define REG_R30         30U  /* 临时/系统寄存器 */
#define REG_R31         31U  /* 临时/系统寄存器 */

/**
 * @brief 常用寄存器别名（提高代码可读性）
 */
#define REG_SP          REG_R3   /* 栈指针 */
#define REG_A0          REG_R4   /* 参数/返回值寄存器0 */
#define REG_A1          REG_R5   /* 参数/返回值寄存器1 */
#define REG_FP          REG_R22  /* 帧指针（s1） */
#define REG_RA          REG_R1   /* 返回地址（某些约定） */

/************************类型定义******************************/
/************************外部变量******************************/
/************************外部函数******************************/

/************************内联函数******************************/

/************************C++兼容性******************************/
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif /* _REGSLOONGARCH_H__ */
