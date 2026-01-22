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
 * @param args  用户参数（a0，将被清零）
 * @param sp    用户栈指针（a1 -> sp）
 * @param entry 用户入口地址（a2 -> ERA）
 * @param prmd  前一个模式寄存器值（PRMD）
 *
 * @note 此函数不会返回
 * @note ertn 是异常返回专用指令，会隐式地将 PRMD 恢复到 CRMD
 * @note 不需要也不应该手动写入 CRMD，ertn 会自动处理
 */
extern void return2user(uintptr_t args, uintptr_t sp, uintptr_t entry, uintptr_t prmd);
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
     *
     * PRMD 寄存器值说明（0x0F）：
     *   - BIT[1:0]  PPLV = 3    (前一个特权级别 = 用户模式)
     *   - BIT[2]    PIE  = 1    (前一个中断使能)
     *   - BIT[3]    PWE  = 1    (前一个等待使能)
     *
     * ertn 指令（异常返回专用指令）的功能：
     *   1. 隐式地将 PRMD 恢复到 CRMD
     *      - PRMD[PPLV] → CRMD[PLV]  (恢复特权级)
     *      - PRMD[PIE]  → CRMD[IE]   (恢复中断使能)
     *      - PRMD[PWE]  → CRMD[DA]   (恢复地址异常使能)
     *   2. 跳转到 ERA 寄存器指向的地址
     *
     * 重要：不需要也不应该手动写入 CRMD，ertn 会自动处理！
     *
     * 参数：
     *   - pcb->args: 传递给用户程序的参数（注意：会被清零）
     *   - pcb->userStack: 用户栈指针
     *   - pcb->entry: 用户程序入口地址
     *   - 0x0F: PRMD 寄存器值，设置 PPLV=3 (用户模式)
     */
    return2user((uintptr_t)pcb->args, (uintptr_t)pcb->userStack, (uintptr_t)pcb->entry, 0x0FU);

    /* 永远不会执行到这里，但为了符合MISRA规范，使用for(;;) */
    for (;;)
    {
        /* 等待，但实际上不会到达这里 */
    }
}
