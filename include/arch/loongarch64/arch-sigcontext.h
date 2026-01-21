/**
 * @file    arch-sigcontext.h
 * @brief   LoongArch64信号上下文定义
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件定义LoongArch64信号处理相关上下文结构
 *          - 信号上下文标志定义
 *          - FPU上下文结构
 *          - LSX（128位SIMD）上下文结构
 *          - LASX（256位SIMD）上下文结构
 *          - LBT（二进制翻译）上下文结构
 *
 * @note MISRA-C:2012 合规
 * @note 用于信号处理时的寄存器状态保存和恢复
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef __ARCH_SIGCONTEXT_H__
#define __ARCH_SIGCONTEXT_H__

/************************头 文 件******************************/
#include <system/const.h>
#include <system/types.h>

/************************宏 定 义******************************/

/**
 * @defgroup SigContextFlags 信号上下文标志
 * @{
 */
#define SC_USED_FP (1U << 0)     /**< 使用了FPU上下文 */
#define SC_ADDRERR_RD (1U << 30) /**< 地址错误由加载引起 */
#define SC_ADDRERR_WR (1U << 31) /**< 地址错误由存储引起 */
/** @} */

/**
 * @defgroup ContextAlign 上下文对齐
 * @{
 */
#define CONTEXT_INFO_ALIGN 16U /**< 上下文信息对齐（字节） */
/** @} */

/************************类型定义******************************/

/**
 * @brief 信号上下文信息头
 *
 * @details 用于标识扩展上下文的类型和大小
 */
struct sctx_info
{
    u32 magic;   /**< 魔数，标识上下文类型 */
    u32 size;    /**< 上下文结构大小 */
    u64 padding; /**< 填充到16字节边界 */
};

/**
 * @defgroup FPUContext FPU上下文定义
 * @{
 */

/**
 * @brief FPU上下文魔数
 */
#define FPU_CTX_MAGIC 0x46505501U

/**
 * @brief FPU上下文对齐
 */
#define FPU_CTX_ALIGN 8U

/**
 * @brief FPU上下文结构
 *
 * @details 保存浮点寄存器和FPU状态
 */
struct fpu_context
{
    u64 regs[32U]; /**< 浮点寄存器$f0-$f31 */
    u64 fcc;       /**< 浮点条件码 */
    u32 fcsr;      /**< 浮点控制状态寄存器 */
};

/** @} */

/**
 * @defgroup LSXContext LSX上下文定义
 * @{
 */

/**
 * @brief LSX上下文魔数
 *
 * @details LSX: LoongArch SIMD Extension（128位SIMD）
 */
#define LSX_CTX_MAGIC 0x53580001U

/**
 * @brief LSX上下文对齐
 */
#define LSX_CTX_ALIGN 16U

/**
 * @brief LSX上下文结构
 *
 * @details 保存LSX 128位SIMD寄存器状态
 */
struct lsx_context
{
    u64 regs[2U * 32U]; /**< LSX寄存器（每个128位，用2个u64表示） */
    u64 fcc;            /**< 浮点条件码 */
    u32 fcsr;           /**< 浮点控制状态寄存器 */
};

/** @} */

/**
 * @defgroup LASXContext LASX上下文定义
 * @{
 */

/**
 * @brief LASX上下文魔数
 *
 * @details LASX: LoongArch Advanced SIMD Extension（256位SIMD）
 */
#define LASX_CTX_MAGIC 0x41535801U

/**
 * @brief LASX上下文对齐
 */
#define LASX_CTX_ALIGN 32U

/**
 * @brief LASX上下文结构
 *
 * @details 保存LASX 256位SIMD寄存器状态
 */
struct lasx_context
{
    u64 regs[4U * 32U]; /**< LASX寄存器（每个256位，用4个u64表示） */
    u64 fcc;            /**< 浮点条件码 */
    u32 fcsr;           /**< 浮点控制状态寄存器 */
};

/** @} */

/**
 * @defgroup LBTContext LBT上下文定义
 * @{
 */

/**
 * @brief LBT上下文魔数
 *
 * @details LBT: LoongArch Binary Translation
 */
#define LBT_CTX_MAGIC 0x42540001U

/**
 * @brief LBT上下文对齐
 */
#define LBT_CTX_ALIGN 8U

/**
 * @brief LBT上下文结构
 *
 * @details 保存二进制翻译相关的寄存器状态
 */
struct lbt_context
{
    u64 regs[4U]; /**< LBT寄存器 */
    u32 eflags;   /**< LBT标志 */
    u32 ftop;     /**< LBT栈顶 */
};

/** @} */

/************************外部变量******************************/
/************************外部函数******************************/
/************************内联函数******************************/
/************************C++兼容性******************************/

#endif /* __ARCH_SIGCONTEXT_H__ */
