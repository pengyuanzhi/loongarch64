/**
 * @file    fpu.h
 * @brief   LoongArch64浮点单元接口
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件定义LoongArch64浮点单元(FPU)相关接口
 *          - FPU状态检测
 *          - FPU使能/禁用控制
 *          - LSX/LASX SIMD扩展控制
 *          - FPU上下文保存/恢复
 *
 * @note MISRA-C:2012 合规
 * @note LoongArch64支持FPU、LSX(128位SIMD)和LASX(256位SIMD)
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef _FPU_H
#define _FPU_H

/************************头 文 件******************************/
#include <cpu-features.h>
#include <cpu-info.h>
#include <cpu.h>
#include <ttos.h>

/************************宏 定 义******************************/
/************************类型定义******************************/
/************************外部变量******************************/
/************************外部函数******************************/

/************************内联函数******************************/

/**
 * @brief 屏蔽FCSR异常原因位
 *
 * @details 根据使能位屏蔽FPU控制状态寄存器的异常原因位
 *
 * @param fcsr FPU控制状态寄存器值
 *
 * @return 返回屏蔽后的FCSR值
 */
static inline unsigned long mask_fcsr_x(unsigned long fcsr)
{
    return fcsr & ((fcsr & FPU_CSR_ALL_E) << (ffs(FPU_CSR_ALL_X) - ffs(FPU_CSR_ALL_E)));
}

/**
 * @brief 检测FPU是否使能
 *
 * @details 检查EUEN寄存器的FPEN位
 *
 * @return 使能返回1，否则返回0
 */
static inline int is_fp_enabled(void)
{
    return (csr_read32(LOONGARCH_CSR_EUEN) & CSR_EUEN_FPEN) ? 1 : 0;
}

/**
 * @brief 检测LSX是否使能
 *
 * @details 检查LSX（128位SIMD）扩展是否使能
 *
 * @return 使能返回1，否则返回0
 */
static inline int is_lsx_enabled(void)
{
    if (!cpu_has_lsx)
    {
        return 0;
    }

    return (csr_read32(LOONGARCH_CSR_EUEN) & CSR_EUEN_LSXEN) ? 1 : 0;
}

/**
 * @brief 检测LASX是否使能
 *
 * @details 检查LASX（256位SIMD）扩展是否使能
 *
 * @return 使能返回1，否则返回0
 */
static inline int is_lasx_enabled(void)
{
    if (!cpu_has_lasx)
    {
        return 0;
    }

    return (csr_read32(LOONGARCH_CSR_EUEN) & CSR_EUEN_LASXEN) ? 1 : 0;
}

/**
 * @brief 检测SIMD是否使能
 *
 * @details 检查LSX或LASX是否使能
 *
 * @return 使能返回1，否则返回0
 */
static inline int is_simd_enabled(void)
{
    return is_lsx_enabled() | is_lasx_enabled();
}

/**
 * @brief 使能FPU
 *
 * @details 设置EUEN寄存器的FPEN位
 */
#define enable_fpu() set_csr_euen(CSR_EUEN_FPEN)

/**
 * @brief 禁用FPU
 *
 * @details 清除EUEN寄存器的FPEN位
 */
#define disable_fpu() clear_csr_euen(CSR_EUEN_FPEN)

/**
 * @defgroup FPUBasic 基础FPU操作
 * @{
 */

/**
 * @brief 保存FPU上下文
 *
 * @details 保存任务的FPU寄存器状态
 *
 * @param tsk 任务控制块指针
 */
static inline void save_fp(TASK_ID tsk)
{
    if (cpu_has_fpu)
    {
        _save_fp(&tsk->switchContext.fpu);
    }
}

/**
 * @brief 恢复FPU上下文
 *
 * @details 恢复任务的FPU寄存器状态
 *
 * @param tsk 任务控制块指针
 */
static inline void restore_fp(TASK_ID tsk)
{
    if (cpu_has_fpu)
    {
        _restore_fp(&tsk->switchContext.fpu);
    }
}

/** @} */

/**
 * @defgroup LSXOPS LSX扩展操作
 * @{
 */

#ifdef CONFIG_CPU_HAS_LSX

/**
 * @brief 使能LSX扩展
 *
 * @details 使能LoongArch SIMD扩展（128位）
 */
static inline void enable_lsx(void)
{
    if (cpu_has_lsx)
    {
        csr_xchg32(CSR_EUEN_LSXEN, CSR_EUEN_LSXEN, LOONGARCH_CSR_EUEN);
    }
}

/**
 * @brief 禁用LSX扩展
 *
 * @details 禁用LoongArch SIMD扩展（128位）
 */
static inline void disable_lsx(void)
{
    if (cpu_has_lsx)
    {
        csr_xchg32(0U, CSR_EUEN_LSXEN, LOONGARCH_CSR_EUEN);
    }
}

/**
 * @brief 保存LSX上下文
 *
 * @param t 任务控制块指针
 */
static inline void save_lsx(struct T_TTOS_TaskControlBlock_Struct *t)
{
    if (cpu_has_lsx)
    {
        _save_lsx(&t->switchContext.fpu);
    }
}

/**
 * @brief 恢复LSX上下文
 *
 * @param t 任务控制块指针
 */
static inline void restore_lsx(struct T_TTOS_TaskControlBlock_Struct *t)
{
    if (cpu_has_lsx)
    {
        _restore_lsx(&t->switchContext.fpu);
    }
}

/**
 * @brief 初始化LSX高位寄存器
 *
 * @details 初始化LSX的高128位寄存器
 */
static inline void init_lsx_upper(void)
{
    if (cpu_has_lsx)
    {
        _init_lsx_upper();
    }
}

/**
 * @brief 恢复LSX高位寄存器
 *
 * @param t 任务控制块指针
 */
static inline void restore_lsx_upper(struct T_TTOS_TaskControlBlock_Struct *t)
{
    if (cpu_has_lsx)
    {
        _restore_lsx_upper(&t->switchContext.fpu);
    }
}

#else

static inline void enable_lsx(void) {}
static inline void disable_lsx(void) {}
static inline void save_lsx(struct T_TTOS_TaskControlBlock_Struct *t)
{
    (void)t;
}
static inline void restore_lsx(struct T_TTOS_TaskControlBlock_Struct *t)
{
    (void)t;
}
static inline void init_lsx_upper(void) {}
static inline void restore_lsx_upper(struct T_TTOS_TaskControlBlock_Struct *t)
{
    (void)t;
}

#endif

/** @} */

/**
 * @defgroup LASXOPS LASX扩展操作
 * @{
 */

#ifdef CONFIG_CPU_HAS_LASX

/**
 * @brief 使能LASX扩展
 *
 * @details 使能LoongArch高级SIMD扩展（256位）
 */
static inline void enable_lasx(void)
{
    if (cpu_has_lasx)
    {
        csr_xchg32(CSR_EUEN_LASXEN, CSR_EUEN_LASXEN, LOONGARCH_CSR_EUEN);
    }
}

/**
 * @brief 禁用LASX扩展
 *
 * @details 禁用LoongArch高级SIMD扩展（256位）
 */
static inline void disable_lasx(void)
{
    if (cpu_has_lasx)
    {
        csr_xchg32(0U, CSR_EUEN_LASXEN, LOONGARCH_CSR_EUEN);
    }
}

/**
 * @brief 保存LASX上下文
 *
 * @param t 任务控制块指针
 */
static inline void save_lasx(struct T_TTOS_TaskControlBlock_Struct *t)
{
    if (cpu_has_lasx)
    {
        _save_lasx(&t->switchContext.fpu);
    }
}

/**
 * @brief 恢复LASX上下文
 *
 * @param t 任务控制块指针
 */
static inline void restore_lasx(struct T_TTOS_TaskControlBlock_Struct *t)
{
    if (cpu_has_lasx)
    {
        _restore_lasx(&t->switchContext.fpu);
    }
}

/**
 * @brief 初始化LASX高位寄存器
 *
 * @details 初始化LASX的高位寄存器
 */
static inline void init_lasx_upper(void)
{
    if (cpu_has_lasx)
    {
        _init_lasx_upper();
    }
}

/**
 * @brief 恢复LASX高位寄存器
 *
 * @param t 任务控制块指针
 */
static inline void restore_lasx_upper(struct T_TTOS_TaskControlBlock_Struct *t)
{
    if (cpu_has_lasx)
    {
        _restore_lasx_upper(&t->switchContext.fpu);
    }
}

#else

static inline void enable_lasx(void) {}
static inline void disable_lasx(void) {}
static inline void save_lasx(struct T_TTOS_TaskControlBlock_Struct *t)
{
    (void)t;
}
static inline void restore_lasx(struct T_TTOS_TaskControlBlock_Struct *t)
{
    (void)t;
}
static inline void init_lasx_upper(void) {}
static inline void restore_lasx_upper(struct T_TTOS_TaskControlBlock_Struct *t)
{
    (void)t;
}

#endif

/** @} */

/************************C++兼容性******************************/
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif /* _FPU_H */
