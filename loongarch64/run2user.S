/**
 * @file    run2user.S
 * @brief   从内核模式切换到用户模式
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 本文件实现从内核模式（PLV0）到用户模式（PLV3）的切换
 *          - 设置用户栈指针
 *          - 设置用户程序入口地址
 *          - 配置 CRMD 寄存器（当前模式寄存器）
 *          - 清空所有寄存器避免信息泄露
 *          - 使用 ertn 指令跳转到用户空间
 *
 * @note MISRA-C:2012 合规
 * @warning 此函数执行后不会返回到内核
 * @warning 调用前必须确保用户栈、参数和入口地址已正确设置
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#define ASM_USE
#include <asm.h>
#include <system/const.h>
#include <cpu.h>
#include <context.h>

/**
 * @brief 切换到用户模式
 *
 * @details 执行从内核模式到用户模式的实际切换
 *          执行流程：
 *          1. 清空参数寄存器 a0（避免信息泄露）
 *          2. 设置用户栈指针（sp = a1）
 *          3. 设置用户入口地址（ERA = a2）
 *          4. 配置 CRMD 寄存器（CRMD = a3）
 *          5. 清空所有通用寄存器（a1-a7, t0-t8, s0-s8）
 *          6. 执行指令和数据同步屏障
 *          7. 使用 ertn 跳转到用户空间
 *
 * @param a0 args      用户参数（将被清零）
 * @param a1 sp        用户栈指针
 * @param a2 entry     用户程序入口地址
 * @param a3 crmd      CRMD 寄存器值（应设置 PLV=3）
 *
 * @return 此函数不会返回
 *
 * @note 此函数必须从内核模式（PLV0）调用
 * @note CRMD 值应为 0x1F 或更大（PLV=3, IE=1, DA=1）
 * @note 执行后处理器将处于用户模式（PLV3）
 * @note 所有寄存器（除了 ERA）都会被清零
 *
 * @par 调用示例
 * @code
 *   // C 代码调用
 *   return2user((uintptr_t)args, (uintptr_t)sp, (uintptr_t)entry, 0x1FU);
 * @endcode
 *
 * @see arch_run2user
 * @see LOONGARCH_CSR_CRMD
 * @see LOONGARCH_CSR_ERA
 */
.global return2user
return2user:
    move        a0,zero                  /* 清空参数寄存器 a0 */
    move        sp,a1                    /* 设置用户栈指针 */
    csrwr       a2,LOONGARCH_CSR_ERA    /* 设置用户入口地址 */
    csrwr       a3,LOONGARCH_CSR_CRMD   /* 设置当前模式寄存器 CRMD */

    /*
     * 清空所有通用寄存器
     * 避免内核信息泄露到用户空间
     */
    move        a1,zero                  /* 清空参数寄存器 a1-a7 */
    move        a2,zero
    move        a3,zero
    move        a4,zero
    move        a5,zero
    move        a6,zero
    move        a7,zero
    move        t0,zero                  /* 清空临时寄存器 t0-t8 */
    move        t1,zero
    move        t2,zero
    move        t3,zero
    move        t4,zero
    move        t5,zero
    move        t6,zero
    move        t7,zero
    move        t8,zero
    move        s0,zero                  /* 清空保存寄存器 s0-s8 */
    move        s1,zero
    move        s2,zero
    move        s3,zero
    move        s4,zero
    move        s5,zero
    move        s6,zero
    move        s7,zero
    move        s8,zero

    ibar        0                        /* 指令同步屏障 */
    dbar        0                        /* 数据同步屏障 */
    ertn                                  /* 异常返回，跳转到用户空间 */
