/**
 * @file    ptrace.h
 * @brief   LoongArch64 架构特定的 ptrace 定义
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 本文件定义 LoongArch64 架构特定的 ptrace 寄存器映射
 *          - 寄存器偏移定义
 *          - CSR 位域定义
 *          - 调试寄存器定义
 *
 * @note MISRA-C:2012 合规
 * @note 此头文件仅适用于 LoongArch64 架构
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef _ARCH_LOONGARCH64_PTRACE_H
#define _ARCH_LOONGARCH64_PTRACE_H

/*************************** 头文件包含 ****************************/
#include <sys/user.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*************************** 宏定义 ****************************/
/**
 * @brief LoongArch64 寄存器数量
 */
#define LOONGARCH64_NUM_REGS 32U
#define LOONGARCH64_NUM_FP_REGS 32U

/**
 * @brief 用户寄存器偏移（用于 ptrace PEEKUSER/POKEUSER）
 */
#define LOONGARCH64_r0 0U
#define LOONGARCH64_r1 1U
#define LOONGARCH64_r2 2U
#define LOONGARCH64_r3 3U
#define LOONGARCH64_r4 4U
#define LOONGARCH64_r5 5U
#define LoongARCH64_r6 6U
#define LOONGARCH64_r7 7U
#define LOONGARCH64_r8 8U
#define LOONGARCH64_r9 9U
#define LOONGARCH64_r10 10U
#define LOONGARCH64_r11 11U
#define LOONGARCH64_r12 12U
#define LOONGARCH64_r13 13U
#define LOONGARCH64_r14 14U
#define LOONGARCH64_r15 15U
#define LOONGARCH64_r16 16U
#define LOONGARCH64_r17 17U
#define LOONGARCH64_r18 18U
#define LOONGARCH64_r19 19U
#define LOONGARCH64_r20 20U
#define LOONGARCH64_r21 21U
#define LOONGARCH64_r22 22U
#define LOONGARCH64_r23 23U
#define LOONGARCH64_r24 24U
#define LOONGARCH64_r25 25U
#define LOONGARCH64_r26 26U
#define LOONGARCH64_r27 27U
#define LOONGARCH64_r28 28U
#define LOONGARCH64_r29 29U
#define LOONGARCH64_r30 30U
#define LOONGARCH64_r31 31U

/**
 * @brief 特殊寄存器偏移
 */
#define LOONGARCH64_pc 32U       /**< @brief 程序计数器 (ERA) */
#define LOONGARCH64_badvaddr 33U /**< @brief 出错地址 */
#define LOONGARCH64_crmd 34U     /**< @brief 当前模式寄存器 */
#define LOONGARCH64_prmd 35U     /**< @brief 前一个模式寄存器 */
#define LOONGARCH64_euen 36U     /**< @brief 扩展单元使能 */
#define LOONGARCH64_ecfg 37U     /**< @brief 异常配置寄存器 */
#define LOONGARCH64_estat 38U    /**< @brief 异常状态寄存器 */

/*************************** CSR 位定义 ****************************/
/**
 * @brief CRMD 寄存器位定义
 */
#define CRMD_PLV (0x3UL << 0)  /**< @brief 特权级别（0=内核, 3=用户） */
#define CRMD_DAIE (0x1UL << 2) /**< @brief 地址异常中断使能 */
#define CRMD_WE (0x1UL << 3)   /**< @brief 执行使能 */
#define CRMD_DATF (0x3UL << 4) /**< @brief 数据访问类型 */
#define CRMD_DATM (0x3UL << 6) /**< @brief 数据访问掩码 */
#define CRMD_DAV (0x1UL << 8)  /**< @brief 数据访问有效性 */

/**
 * @brief PRMD 寄存器位定义
 */
#define PRMD_PPLV (0x3UL << 0) /**< @brief 前一个特权级别 */
#define PRMD_DATF (0x3UL << 4) /**< @brief 前一个数据访问类型 */
#define PRMD_DATM (0x3UL << 6) /**< @brief 前一个数据访问掩码 */
#define PRMD_DAV (0x1UL << 8)  /**< @brief 前一个数据访问有效性 */

/**
 * @brief EUEN 寄存器位定义
 */
#define EUEN_FPE (0x1UL << 0)   /**< @brief 浮点单元使能 */
#define EUEN_SXE (0x1UL << 1)   /**< @brief SIMD 扩展使能 */
#define EUEN_ASXE (0x1UL << 2)  /**< @brief 高级 SIMD 扩展使能 */
#define EUEN_LBTPE (0x1UL << 3) /**< @brief 轻量级事务处理使能 */

/**
 * @brief ECFG 寄存器位定义
 */
#define ECFG_LIE (0x1FFFUL << 0) /**< @brief 局部中断使能 */

/**
 * @brief ESTAT 寄存器位定义
 */
#define ESTAT_ECODE (0x3FUL << 0)      /**< @brief 异常码 */
#define ESTAT_ESUBCODE (0x3FFUL << 16) /**< @brief 异常子码 */
#define ESTAT_IS (0x1UL << 2)          /**< @brief 中断栈 */

/*************************** 调试寄存器定义 ****************************/
/**
 * @brief LoongArch64 调试 CSR 寄存器
 *
 * @note LoongArch64 调试支持使用以下 CSR 寄存器：
 *       - PRCTL (0x80): 调试控制寄存器
 *       - PRB[0-15] (0xA0-0xAF): 断点地址寄存器
 *       - PRM[0-15] (0xB0-0xBF): 断点掩码寄存器
 *       - PRB[0-15]_CTRL (0xC0-0xCF): 断点控制寄存器
 *       - PRDATA[0-15] (0xD0-0xDF): 观察点数据寄存器
 *       - PRDATA[0-15]_CTRL (0xE0-0xEF): 观察点控制寄存器
 */

/**
 * @brief PRCTL 寄存器位定义
 */
#define PRCTL_SS (0x1UL << 0)  /**< @brief 单步使能 */
#define PRCTL_STE (0x1UL << 1) /**< @brief 单步触发使能 */
#define PRCTL_ITE (0x1UL << 2) /**< @brief 指令跟踪使能 */

/**
 * @brief 断点/观察点控制寄存器位定义
 */
#define DBG_CTRL_EN (0x1UL << 0)    /**< @brief 断点/观察点使能 */
#define DBG_CTRL_MASK (0x3FUL << 2) /**< @brief 地址掩码 */

/**
 * @brief 断点类型定义
 */
#define DBG_BPT_INST 0U  /**< @brief 指令断点 */
#define DBG_BPT_DATA 1U  /**< @brief 数据断点 */
#define DBG_BPT_RDWR 2U  /**< @brief 读写断点 */
#define DBG_BPT_WRITE 3U /**< @brief 写断点 */

    /*************************** 类型定义 ****************************/
    /**
     * @brief LoongArch64 特定的用户上下文
     *
     * @details 扩展标准 user 结构，添加架构特定字段
     */
    struct loongarch64_user_context
    {
        struct user regs; /**< @brief 通用寄存器和 FPU 寄存器 */
        uint64_t orig_r4; /**< @brief 原始系统调用参数 (r4/a0) */
    };

    /*************************** 内联函数 ****************************/
    /**
     * @brief 获取用户寄存器结构中的 PC
     *
     * @param regs 用户寄存器结构指针
     *
     * @return PC 值
     */
    static inline uint64_t ptrace_get_pc(struct user_regs_struct *regs)
    {
        return regs->csr_era;
    }

    /**
     * @brief 设置用户寄存器结构中的 PC
     *
     * @param regs 用户寄存器结构指针
     * @param pc 新的 PC 值
     */
    static inline void ptrace_set_pc(struct user_regs_struct *regs, uint64_t pc)
    {
        regs->csr_era = pc;
    }

    /**
     * @brief 获取用户寄存器结构中的 SP
     *
     * @param regs 用户寄存器结构指针
     *
     * @return SP 值 (r3)
     */
    static inline uint64_t ptrace_get_sp(struct user_regs_struct *regs)
    {
        return regs->regs[3]; /* r3 是栈指针 */
    }

    /**
     * @brief 设置用户寄存器结构中的 SP
     *
     * @param regs 用户寄存器结构指针
     * @param sp 新的 SP 值
     */
    static inline void ptrace_set_sp(struct user_regs_struct *regs, uint64_t sp)
    {
        regs->regs[3] = sp;
    }

    /**
     * @brief 检查是否处于用户模式
     *
     * @param regs 用户寄存器结构指针
     *
     * @return 在用户模式返回 true，否则返回 false
     */
    static inline bool ptrace_is_user_mode(struct user_regs_struct *regs)
    {
        /* PLV=3 表示用户模式 */
        return ((regs->csr_crmd & CRMD_PLV) == 0x3UL);
    }

    /**
     * @brief 启用单步执行
     *
     * @param regs 用户寄存器结构指针
     */
    static inline void ptrace_enable_single_step(struct user_regs_struct *regs)
    {
        /* 使用 CRMD 的 DAIE 位进行单步调试 */
        regs->csr_crmd |= CRMD_DAIE;
    }

    /**
     * @brief 禁用单步执行
     *
     * @param regs 用户寄存器结构指针
     */
    static inline void ptrace_disable_single_step(struct user_regs_struct *regs)
    {
        regs->csr_crmd &= ~CRMD_DAIE;
    }

    /**
     * @brief 检查是否启用单步执行
     *
     * @param regs 用户寄存器结构指针
     *
     * @return 启用返回 true，否则返回 false
     */
    static inline bool ptrace_is_single_step(struct user_regs_struct *regs)
    {
        return ((regs->csr_crmd & CRMD_DAIE) != 0x0UL);
    }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ARCH_LOONGARCH64_PTRACE_H */
