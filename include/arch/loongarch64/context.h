/**
 * @file    context.h
 * @brief   LoongArch64上下文定义
 * @author  AISafe64 Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件定义了LoongArch64处理器上下文相关数据结构
 *          - 异常/中断上下文（arch_context）
 *          - 任务切换上下文（task_context）
 *          - 系统调用/异常寄存器保存（pt_regs）
 *          - FPU上下文定义
 *
 * @note MISRA-C:2012 合规
 * @warning 上下文结构必须与汇编代码中的偏移量一致
 *
 * @copyright Copyright (c) 2025 AISafe64 Team
 */

#ifndef _CONTEXT_H
#define _CONTEXT_H

/************************头 文 件******************************/
#include <system/types.h>
#include <fpregdef.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/************************宏 定 义******************************/

/**
 * @brief 线程信息块大小
 */
#define THREAD_SIZE             0x00004000UL

/**
 * @brief 线程掩码（用于对齐）
 */
#define THREAD_MASK             (THREAD_SIZE - 1UL)

/************************类型定义******************************/

#ifndef ASM_USE

#ifdef _HARD_FLOAT_

/**
 * @brief FPU寄存器联合体
 *
 * @details 支持以32位或64位方式访问FPU寄存器
 */
union fpureg
{
    u32 val32[FPU_REG_WIDTH / 32U];  /**< 32位访问方式 */
    u64 val64[FPU_REG_WIDTH / 64U];  /**< 64位访问方式 */
};

/**
 * @brief FPU上下文结构
 *
 * @details 保存完整的浮点单元状态
 */
typedef struct
{
    u64 fcc;                    /**< 浮点条件码（8x8位） */
    u32 fcsr;                   /**< 浮点控制和状态寄存器 */
    u32 ftop;                   /**< 浮点栈顶指针 */
    union fpureg fpr[32];       /**< 32个浮点寄存器 */
} T_FPP_Context __attribute__((__aligned__(8)));

/**
 * @brief LoongArch FPU状态结构
 *
 * @details 用于保存和恢复FPU状态
 */
struct loongarch_fpu
{
    u64 fcc;                    /**< 浮点条件码（8x8位） */
    u32 fcsr;                   /**< 浮点控制和状态寄存器 */
    u32 ftop;                   /**< 浮点栈顶指针 */
    union fpureg fpr[32];       /**< 32个浮点寄存器 */
};

#endif /* _HARD_FLOAT_ */

/**
 * @brief 异常/中断上下文结构
 *
 * @details 保存异常或中断发生时的处理器状态
 *          用于异常处理和系统调用
 */
struct arch_context
{
    /* 主处理器寄存器 */
    u64 regs[32];               /**< 通用寄存器r0-r31 */

    /* 系统调用原始参数 */
    u64 orig_a0;                /**< 系统调用原始参数a0 */

    /* 上下文类型 */
    u64 type;                   /**< 上下文类型（IRQ/SYSCALL等） */

    /* CSR特殊寄存器 */
    u64 csr_era;                /**< 异常返回地址 */
    u64 csr_badvaddr;           /**< 出错虚拟地址 */
    u64 csr_crmd;               /**< 当前模式信息 */
    u64 csr_prmd;               /**< 前一个异常模式信息 */
    u64 csr_euen;               /**< 扩展单元使能 */
    u64 csr_ecfg;               /**< 异常配置 */
    u64 csr_estat;              /**< 异常状态 */

    /* 可变长度数组（用于扩展） */
    u64 __last[];               /**< 扩展字段 */
} __attribute__((__aligned__(8)));

/**
 * @brief 中断上下文类型
 */
typedef struct arch_context arch_int_context_t;

/**
 * @brief 异常上下文类型
 */
typedef struct arch_context arch_exception_context_t;

/**
 * @brief 任务切换上下文结构
 *
 * @details 保存任务切换时的完整上下文状态
 *          用于任务调度和上下文切换
 */
struct task_context
{
    /* CSR特殊寄存器 */
    u64 csr_badvaddr;           /**< 出错虚拟地址 */
    u64 csr_crmd;               /**< 当前模式信息 */
    u64 csr_prmd;               /**< 前一个异常模式信息 */
    u64 csr_euen;               /**< 扩展单元使能 */
    u64 csr_ecfg;               /**< 异常配置 */
    u64 csr_estat;              /**< 异常状态 */

    /* 程序计数器和栈指针 */
    u64 pc;                     /**< 程序计数器 */
    u64 sp;                     /**< 栈指针 */

    /* 线程本地存储和进程空间 */
    u64 tls;                    /**< POSIX线程上下文 */
    u64 pgdl;                   /**< 用户进程页表基址（VA[VALEN-1]=0） */
    u64 asid;                   /**< 地址空间标识符 */

    /* 虚拟中断 */
    u64 vintflag;               /**< 虚拟中断标志位 */

    /* 主处理器寄存器 */
    u64 regs[32];               /**< 通用寄存器r0-r31 */

#ifdef _HARD_FLOAT_
    /* FPU状态 */
    struct loongarch_fpu fpu __attribute__((aligned(32))); /**< FPU上下文 */
#endif
};

/**
 * @brief 任务上下文类型别名
 */
typedef struct task_context T_TBSP_TaskContext;

/**
 * @brief 系统调用/异常寄存器保存结构
 *
 * @details 定义系统调用或异常发生时寄存器在栈上的存储格式
 *          与arch_context结构类似，但使用unsigned long类型
 */
struct pt_regs
{
    /* 主处理器寄存器 */
    unsigned long regs[32];     /**< 通用寄存器r0-r31 */

    /* 原始系统调用参数 */
    unsigned long orig_a0;      /**< 系统调用原始参数a0 */

    /* CSR特殊寄存器 */
    unsigned long csr_era;      /**< 异常返回地址 */
    unsigned long csr_badvaddr; /**< 出错虚拟地址 */
    unsigned long csr_crmd;     /**< 当前模式信息 */
    unsigned long csr_prmd;     /**< 前一个异常模式信息 */
    unsigned long csr_euen;     /**< 扩展单元使能 */
    unsigned long csr_ecfg;     /**< 异常配置 */
    unsigned long csr_estat;    /**< 异常状态 */

    /* 可变长度数组（用于扩展） */
    unsigned long __last[];     /**< 扩展字段 */
} __attribute__((__aligned__(8)));

#endif /* !ASM_USE */

/************************C++兼容性******************************/
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _CONTEXT_H */
