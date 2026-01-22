/**
 * @file    int.h
 * @brief   LoongArch64中断控制接口
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件定义LoongArch64中断控制相关接口
 *          - 本地中断使能/禁用
 *          - 中断标志保存/恢复
 *          - 中断状态检测
 *          - 使用CRMD寄存器的IE位控制中断
 *
 * @note MISRA-C:2012 合规
 * @note 使用csrxchg指令原子地修改CRMD寄存器
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef _INT_H
#define _INT_H

/************************头 文 件******************************/
#include <system/types.h>

/************************宏 定 义******************************/

/**
 * @brief 中断使能寄存器位定义
 */
#define REG_INT_ENABLE 0x00000004U

/************************类型定义******************************/
/************************外部变量******************************/
/************************外部函数******************************/

/************************内联函数******************************/

/**
 * @defgroup IRQControl 本地中断控制
 * @{
 */

/**
 * @brief 使能本地中断
 *
 * @details 设置CRMD寄存器的IE位（位2），使能处理器中断
 */
static inline void arch_local_irq_enable(void)
{
    unsigned int flags = (0x1U << 2U);
    __asm__ __volatile__("csrxchg %[val], %[mask], %[reg]\n\t"
                         : [val] "+r"(flags)
                         : [mask] "r"(0x1U << 2U), [reg] "i"(0x0)
                         : "memory");
}

/**
 * @brief 禁用本地中断
 *
 * @details 清除CRMD寄存器的IE位（位2），禁用处理器中断
 */
static inline void arch_local_irq_disable(void)
{
    unsigned int flags = 0U;
    __asm__ __volatile__("csrxchg %[val], %[mask], %[reg]\n\t"
                         : [val] "+r"(flags)
                         : [mask] "r"(0x1U << 2U), [reg] "i"(0x0)
                         : "memory");
}

/**
 * @brief 保存中断标志并禁用中断
 *
 * @details 保存当前中断状态到flags，然后禁用中断
 *
 * @return 返回保存的中断标志
 */
static inline unsigned long arch_local_irq_save(void)
{
    unsigned int flags = 0U;
    __asm__ __volatile__("csrxchg %[val], %[mask], %[reg]\n\t"
                         : [val] "+r"(flags)
                         : [mask] "r"(0x1U << 2U), [reg] "i"(0x0)
                         : "memory");
    return (unsigned long)flags;
}

/**
 * @brief 恢复中断标志
 *
 * @details 从flags恢复之前保存的中断状态
 *
 * @param flags 保存的中断标志
 */
static inline void arch_local_irq_restore(unsigned long flags)
{
    __asm__ __volatile__("csrxchg %[val], %[mask], %[reg]\n\t"
                         : [val] "+r"(flags)
                         : [mask] "r"(0x1U << 2U), [reg] "i"(0x0)
                         : "memory");
}

/**
 * @brief 获取当前中断标志
 *
 * @details 读取CRMD寄存器并返回中断使能位
 *
 * @return 返回CRMD寄存器的值
 */
static inline unsigned long arch_local_save_flags(void)
{
    unsigned int flags;
    __asm__ __volatile__("csrrd %[val], %[reg]\n\t" : [val] "=r"(flags) : [reg] "i"(0x0) : "memory");
    return (unsigned long)flags;
}

/**
 * @brief 检查中断标志是否被禁用
 *
 * @details 检查flags中的IE位是否为0
 *
 * @param flags 中断标志
 *
 * @return 中断禁用返回非0，否则返回0
 */
static inline int arch_irqs_disabled_flags(unsigned long flags)
{
    return (int)!(flags & (0x1UL << 2UL));
}

/**
 * @brief 检查当前中断是否被禁用
 *
 * @details 检查当前处理器中断是否被禁用
 *
 * @return 中断禁用返回非0，否则返回0
 */
static inline int arch_irqs_disabled(void)
{
    return arch_irqs_disabled_flags(arch_local_save_flags());
}

/** @} */

/**
 * @defgroup IRQWrappers 中断控制包装宏
 * @{
 */

/**
 * @brief CPU中断使能宏
 */
#define arch_cpu_int_enable()    \
    do                           \
    {                            \
        arch_local_irq_enable(); \
    } while (0)

/**
 * @brief CPU中断禁用宏
 */
#define arch_cpu_int_disable()    \
    do                            \
    {                             \
        arch_local_irq_disable(); \
    } while (0)

/**
 * @brief 保存中断标志宏
 */
#define raw_local_irq_save(flags)      \
    do                                 \
    {                                  \
        flags = arch_local_irq_save(); \
    } while (0)

/**
 * @brief 恢复中断标志宏
 */
#define raw_local_irq_restore(flags)   \
    do                                 \
    {                                  \
        arch_local_irq_restore(flags); \
    } while (0)

/**
 * @brief 保存中断标志到变量宏
 */
#define raw_local_save_flags(flags)      \
    do                                   \
    {                                    \
        flags = arch_local_save_flags(); \
    } while (0)

/**
 * @brief 检查中断标志是否禁用宏
 */
#define raw_irqs_disabled_flags(flags) ({ arch_irqs_disabled_flags(flags); })

/**
 * @brief 禁止CPU中断宏
 */
#define arch_cpu_int_save(flags)   \
    do                             \
    {                              \
        raw_local_irq_save(flags); \
    } while (0)

/**
 * @brief 恢复CPU中断宏
 */
#define arch_cpu_int_restore(flags)   \
    do                                \
    {                                 \
        raw_local_irq_restore(flags); \
    } while (0)

/** @} */

/************************C++兼容性******************************/
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif /* _INT_H */
