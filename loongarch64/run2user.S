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
 *          - 配置 PRMD 寄存器（前一个模式寄存器）
 *          - 使用 ertn 指令（异常返回专用指令）
 *          - 清空所有寄存器避免信息泄露
 *          - 跳转到用户空间执行
 *
 * @note ertn 是异常返回专用指令，会隐式地将 PRMD 恢复到 CRMD
 * @note 不需要也不应该手动写入 CRMD，ertn 会自动处理
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
 *          1. 设置用户栈指针（sp = a1）
 *          2. 设置用户入口地址（ERA = a2）
 *          3. 配置 PRMD 寄存器（PRMD = a3）
 *          4. 清空所有通用寄存器（a0-a7, t0-t8, s0-s8, fp, tp）
 *          5. 执行指令和数据同步屏障
 *          6. 执行 ertn 指令（异常返回专用指令）
 *
 * @param a0 args      用户参数（将被清零，参数通过 auxv 在栈上传递）
 * @param a1 sp        用户栈指针
 * @param a2 entry     用户程序入口地址
 * @param a3 prmd      PRMD 寄存器值（应设置 PPLV=3 表示用户模式）
 *
 * @return 此函数不会返回
 *
 * @note 此函数必须从内核模式（PLV0）调用
 * @note 用户程序参数（argc, argv, envp）通过 auxv 在用户栈上传递，不使用寄存器
 * @note ertn 是异常返回专用指令，会隐式地将 PRMD 恢复到 CRMD
 * @note ertn 的功能：
 *       - 自动将 PRMD[PPLV] 复制到 CRMD[PLV]，恢复特权级
 *       - 自动将 PRMD[PIE] 复制到 CRMD[IE]，恢复中断使能状态
 *       - 自动将 PRMD[PWE] 复制到 CRMD[DA]，恢复地址异常使能状态
 *       - 跳转到 ERA 寄存器指向的地址
 * @note 不需要也不应该手动写入 CRMD，ertn 会自动处理
 * @note PRMD 值应设置 PPLV=3（用户模式）、PIE=1（中断使能）、PWE=1（等待使能）
 * @note 执行 ertn 后，处理器将处于用户模式（PLV3），CRMD = PRMD 的值
 * @note 所有通用寄存器都会被清零以避免内核信息泄露
 */
.global return2user
return2user:
    move        sp,a1                    /* 设置用户栈指针 */
    csrwr       a2,LOONGARCH_CSR_ERA     /* 设置用户入口地址 */
    csrwr       a3,LOONGARCH_CSR_PRMD    /* 设置前一个模式寄存器 PRMD */

    /*
     * 清空所有通用寄存器
     * 避免内核信息泄露到用户空间
     */
    move        a0,zero                  /* 清空参数寄存器 a0-a7 (r4-r11) */
    move        a1,zero
    move        a2,zero
    move        a3,zero
    move        a4,zero
    move        a5,zero
    move        a6,zero
    move        a7,zero
    move        t0,zero                  /* 清空临时寄存器 t0-t8 (r12-r20) */
    move        t1,zero
    move        t2,zero
    move        t3,zero
    move        t4,zero
    move        t5,zero
    move        t6,zero
    move        t7,zero
    move        t8,zero
    move        fp,zero                  /* 清空帧指针 r22 */
    move        tp,zero                  /* 清空线程指针 r2 */
    move        ra,zero                  /* 清空返回地址 r1 */
    move        s0,zero                  /* 清空保存寄存器 s0-s8 (r23-r31) */
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
    ertn                                 /* 异常返回：隐式地将 PRMD 恢复到 CRMD，跳转到 ERA */
