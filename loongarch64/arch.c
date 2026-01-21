/**
 * @file    arch.c
 * @brief   LoongArch64架构上下文管理实现
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件实现了LoongArch64架构上下文管理相关功能
 *          - 上下文切换
 *          - 异常上下文管理
 *          - 函数参数获取
 *          - 线程上下文初始化
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

/*************************** 头文件包含 ****************************/

#include <ttos_arch.h>
#include <cpu.h>
#include <ttos.h>
#include <ttosProcess.h>

/*************************** 宏定义 *****************************/

#define REG_A0          4U   /* 参数寄存器0 / 返回值寄存器0 */
#define REG_SP          3U   /* 堆栈指针寄存器 */
#define MAX_REGS        32U  /* r0-r31，共32个通用寄存器 */

/*************************** 函数实现 ****************************/

/**
 * @brief 设置任务上下文的堆栈指针
 *
 * @param ctx 任务上下文指针
 * @param sp  要设置的堆栈指针值
 *
 * @return 成功返回0
 */
int32_t arch_switch_context_set_stack(T_TBSP_TaskContext *ctx, uint64_t sp)
{
    ctx->sp = sp;

    return 0;
}

/**
 * @brief 设置异常上下文的返回值
 *
 * @param context 异常上下文指针
 * @param value   要设置的返回值
 *
 * @return 成功返回0
 */
int32_t arch_context_set_return(arch_exception_context_t *context,
                               uint64_t value)
{
    /* a0是函数返回值寄存器（r4） */
    context->regs[REG_A0] = value;

    return 0;
}

/**
 * @brief 设置异常上下文的堆栈指针
 *
 * @param context 异常上下文指针
 * @param value   要设置的堆栈指针值
 *
 * @return 成功返回0
 */
int32_t arch_context_set_stack(arch_exception_context_t *context,
                              uint64_t value)
{
    /* sp（r3）寄存器用于堆栈指针 */
    context->regs[REG_SP] = value;

    return 0;
}

/**
 * @brief 初始化线程上下文
 *
 * @param context 异常上下文指针
 *
 * @return 成功返回0
 */
int32_t arch_context_thread_init(arch_exception_context_t *context)
{
    return 0;
}

/**
 * @brief 获取寄存器值
 *
 * @details 从异常上下文中获取指定寄存器的值
 *
 * @param context 异常上下文指针
 * @param index   寄存器索引（0-31，对应r0-r31）
 *
 * @return 成功返回寄存器值，失败返回-1
 *
 * @note LoongArch64通用寄存器：
 *       - r0: 零寄存器（硬编码为0）
 *       - r1: 保留
 *       - r2-r3: 栈指针和全局指针
 *       - r4-r11: 参数寄存器和返回值（a0-a7）
 *       - r12-r20: 临时寄存器（t0-t8）
 *       - r21-r31: 保存寄存器（s0-s8）和特殊寄存器
 */
int64_t arch_context_get_args(arch_exception_context_t *context,
                               uint32_t index)
{
    if (index < MAX_REGS)
    {
        return context->regs[index];
    }

    return -1;
}

/**
 * @brief 设置寄存器值
 *
 * @details 设置异常上下文中的指定寄存器值
 *
 * @param context 异常上下文指针
 * @param index   寄存器索引（0-31，对应r0-r31）
 * @param value   要设置的寄存器值
 *
 * @return 成功返回0，失败返回-1
 *
 * @note 用于系统调用或信号处理时设置寄存器值
 */
int32_t arch_context_set_args(arch_exception_context_t *context,
                               uint32_t index,
                               uint64_t value)
{
    if (index < MAX_REGS)
    {
        context->regs[index] = value;

        return 0;
    }

    return -1;
}
