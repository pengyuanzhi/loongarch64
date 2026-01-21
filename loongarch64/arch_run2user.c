/**
 * @file    arch_run2user.c
 * @brief   LoongArch64用户模式切换
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件实现从内核模式切换到用户模式的功能
 *          - 用户模式入口设置
 *          - 用户栈配置
 *          - 特权级切换
 *
 * @note MISRA-C:2012 合规
 * @warning 此函数执行后不会返回到内核
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */
/*************************** 头文件包含 ****************************/
#include <assert.h>
#include <cpu.h>
#include <stdint.h>
#include <system/compiler.h>
#include <ttos.h>
#include <ttosProcess.h>
/*************************** 外部函数声明 ****************************/
/**
 * @brief 重置调试状态
 *
 * @details 清除调试寄存器和断点状态
 */
void reset_debug_state(void);
/**
 * @brief 切换到用户模式
 *
 * @details 汇编函数，执行从内核模式到用户模式的实际切换
 *
 * @param args  用户参数
 * @param sp    用户栈指针
 * @param entry 用户入口地址
 * @param msr   机器状态寄存器值
 *
 * @note 此函数不会返回
 */
extern void return2user(uintptr_t args, uintptr_t sp, uintptr_t entry, uintptr_t msr);
/*************************** 函数实现 ****************************/
/**
 * @brief 从内核模式切换到用户模式
 *
 * @details 将处理器从特权级PLV0（内核模式）切换到PLV3（用户模式）
 *          设置用户栈、参数和入口地址，然后跳转到用户空间执行
 *
 * @param 无
 *
 * @return 此函数不会返回
 *
 * @note 函数标记为__noreturn，编译器知道此函数不会返回
 * @note 执行流程：
 *       1. 获取当前进程的PCB
 *       2. 重置调试状态
 *       3. 调用用户模式钩子
 *       4. 调用汇编函数完成切换
 *
 * @warning 调用前必须确保用户栈、参数和入口地址已正确设置
 */
__noreturn void arch_run2user(void)
{
    pcb_t pcb = ttosProcessSelf();

    /* 断言：进程控制块必须有效 */
    assert(pcb != NULL);

    /* 重置调试状态 */
    reset_debug_state();

    /* 进入用户模式前的钩子函数 */
    TTOS_TaskEnterUserHook(TTOS_GetRunningTask());

    /*
     * 调用汇编函数切换到用户模式
     * 参数：
     *   - pcb->args: 传递给用户程序的参数
     *   - pcb->userStack: 用户栈指针
     *   - pcb->entry: 用户程序入口地址
     *   - 0xb7: CSR值，设置PLV3（用户模式）
     */
    return2user((uintptr_t)pcb->args, (uintptr_t)pcb->userStack, (uintptr_t)pcb->entry, 0xb7U);

    /* 永远不会执行到这里，但为了符合MISRA规范，使用for(;;) */
    for (;;)
    {
        /* 等待，但实际上不会到达这里 */
    }
}
