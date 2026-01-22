/**
 * @file    user.h
 * @brief   LoongArch64 用户寄存器结构定义
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 本文件定义了 LoongArch64 架构的用户寄存器结构
 *          用于 ptrace 调试接口
 *          - user_regs_struct：通用寄存器
 *          - user_fpsimd_struct：浮点/SIMD 寄存器
 *          - user_hwdebug_state：硬件调试状态
 *
 * @note MISRA-C:2012 合规
 * @note 结构定义必须与内核 pt_regs 保持一致
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef _SYS_USER_H
#define _SYS_USER_H

/*************************** 头文件包含 ****************************/
#include <system/types.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*************************** 宏定义 ****************************/
/**
 * @defgroup LoongArchRegisters LoongArch64 寄存器数量
 * @{
 */
#define LOONGARCH_NUM_REGS 32U    /**< @brief 通用寄存器数量 */
#define LOONGARCH_NUM_FP_REGS 32U /**< @brief 浮点寄存器数量 */
/** @} */

/*************************** 数据结构 ****************************/
/**
 * @brief LoongArch64 用户寄存器结构
 *
 * @details 包含通用寄存器和关键 CSR 寄存器
 *          用于调试器查看和修改进程状态
 */
struct user_regs_struct
{
    /* 通用寄存器 r0-r31 */
    uint64_t regs[LOONGARCH_NUM_REGS]; /**< @brief 通用寄存器数组 */

    /* CSR 特殊寄存器 */
    uint64_t csr_era;      /**< @brief 异常返回地址（程序计数器） */
    uint64_t csr_badvaddr; /**< @brief 出错虚拟地址 */
    uint64_t csr_crmd;     /**< @brief 当前模式寄存器 */
    uint64_t csr_prmd;     /**< @brief 前一个异常模式寄存器 */
    uint64_t csr_euen;     /**< @brief 扩展单元使能寄存器 */
    uint64_t csr_ecfg;     /**< @brief 异常配置寄存器 */
    uint64_t csr_estat;    /**< @brief 异常状态寄存器 */
};

/**
 * @brief LoongArch64 浮点寄存器结构
 *
 * @details 包含 FPU 状态和浮点寄存器
 */
struct user_fpsimd_struct
{
    /* 浮点控制寄存器 */
    uint64_t fcc;  /**< @brief 浮点条件码（8x8位） */
    uint32_t fcsr; /**< @brief 浮点控制和状态寄存器 */
    uint32_t ftop; /**< @brief 浮点栈顶指针 */

    /* 浮点数据寄存器 fpr0-fpr31 */
    uint64_t fpr[LOONGARCH_NUM_FP_REGS]; /**< @brief 浮点寄存器数组 */
};

/**
 * @brief 硬件调试寄存器结构
 *
 * @details 用于硬件断点和观察点
 */
struct dbg_reg
{
    uint64_t addr; /**< @brief 断点/观察点地址 */
    uint32_t ctrl; /**< @brief 控制寄存器 */
    uint32_t pad;  /**< @brief 填充 */
};

/**
 * @brief 硬件调试状态结构
 *
 * @details 包含所有硬件断点和观察点信息
 */
struct user_hwdebug_state
{
    uint32_t dbg_info;           /**< @brief 调试信息（架构版本、寄存器数量） */
    uint32_t pad;                /**< @brief 填充对齐 */
    struct dbg_reg dbg_regs[16]; /**< @brief 调试寄存器数组 */
};

/*************************** 类型定义 ****************************/
/**
 * @brief 用户状态结构
 *
 * @details 包含通用寄存器和浮点寄存器的完整状态
 */
struct user
{
    struct user_regs_struct user_regs;     /**< @brief 通用寄存器 */
    struct user_fpsimd_struct user_fpsimd; /**< @brief 浮点寄存器 */
};

/*************************** CSR 位定义 ****************************/
/**
 * @defgroup CRMD_Bits CRMD 寄存器位定义
 * @{
 */
#define CRMD_PLV (0x3UL << 0)  /**< @brief 特权级别（0=内核, 3=用户） */
#define CRMD_DAIE (0x1UL << 2) /**< @brief 地址异常中断使能 */
#define CRMD_WE (0x1UL << 3)   /**< @brief 执行使能 */
/** @} */

/**
 * @defgroup PRMD_Bits PRMD 寄存器位定义
 * @{
 */
#define PRMD_PPLV (0x3UL << 0) /**< @brief 前一个特权级别 */
/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _SYS_USER_H */
