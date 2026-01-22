/**
 * @file    entry.S
 * @brief   LoongArch64 系统调用处理入口
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 本文件实现 LoongArch64 架构的系统调用处理入口
 *          - 系统调用入口函数 (handle_syscall)
 *          - 上下文保存和恢复
 *          - 从用户栈切换到内核栈
 *          - 保存寄存器状态（通用寄存器、CSR 寄存器）
 *          - 调用 C 语言系统调用处理函数 (do_syscall)
 *          - 恢复上下文并返回用户空间
 *
 * @note MISRA-C:2012 合规
 * @warning 系统调用必须在用户模式（PLV3）下触发
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

/*************************** 修改历史 ****************************/
/*
 * 修改历史：
 * 2025-01-22    Intewell Team
 *               创建该文件
 */

/*************************** 头文件包含 ****************************/
#define ASM_USE
#include <cpu.h>
#include <asm.h>
#include <asrspace.h>

/*************************** 宏定义 ****************************/


/*************************** 类型定义 ****************************/


/*************************** 外部声明 ****************************/
    .extern handle_syscall
    .extern do_syscall
    .extern running_task_stack

/*************************** 前向声明 ****************************/


/*************************** 模块变量 ****************************/


/*************************** 函数实现 ****************************/

/**
 * @brief 系统调用处理入口函数
 *
 * @details 处理用户空间发起的系统调用
 *          执行流程：
 *          1. 保存用户栈指针到 t2
 *          2. 切换到内核栈（SMP：根据 CPU ID 计算）
 *          3. 在内核栈上分配异常上下文空间（PT_SIZE）
 *          4. 保存所有通用寄存器（r0-r31）
 *          5. 保存 CSR 寄存器（ERA、CRMD、PRMD、EUEN、ECFG、ESTAT）
 *          6. 调用 C 语言系统调用处理函数 do_syscall
 *          7. 恢复所有寄存器并返回用户空间
 *
 * @note 系统调用来自用户空间，需要切换到内核栈
 * @note t0-t8 寄存器（r12-r20）是调用者保存，不需要在此保存
 * @note 系统调用号通过 a7 寄存器（r11）传递
 * @note 系统调用参数通过 a0-a6 寄存器（r4-r10）传递
 * @note 返回值通过 a0 寄存器（r4）返回
 *
 * @param a0-a6  系统调用参数（最多 7 个参数）
 * @param a7     系统调用号
 *
 * @return 系统调用返回值（通过 a0 寄存器）
 *
 * @note 此函数由硬件 syscall 指令触发
 * @note 执行环境：从用户模式（PLV3）进入内核模式（PLV0）
 * @note 返回环境：从内核模式（PLV0）返回用户模式（PLV3）
 *
 * @warning 必须确保内核栈已正确初始化
 * @warning 调用 do_syscall 后必须立即恢复上下文
 */
    .text
    .align  5
ENTRY(handle_syscall)
    /* 保存用户栈指针 */
    move    t2, sp

#ifdef CONFIG_SMP
    /* 获取当前 CPU ID */
    csrrd   t0, LOONGARCH_CSR_CPUID
    li.w    t1, __SIZEOF_POINTER__
    mul.w   t0, t0, t1
#endif

    /* 获取当前任务的内核栈地址 */
    la_abs  t1, running_task_stack

#ifdef CONFIG_SMP
    /* SMP：根据 CPU ID 计算栈地址 */
    LONG_ADD    t1, t1, t0
#endif

    /* 切换到内核栈 */
    LONG_L  sp, t1, 0

    /* 在内核栈上分配异常上下文空间 */
    addi.d  sp, sp, -PT_SIZE

    /*
     * 保存通用寄存器
     */

    /* r0：常量 0 寄存器 */
    LONG_S  zero, sp, PT_R0

    /* r1：返回地址寄存器（发起系统调用的函数的返回地址） */
    LONG_S  ra, sp, PT_R1

    /* r2：线程指针寄存器（发起系统调用的线程的指针） */
    LONG_S  tp, sp, PT_R2

    /* r3：之前的栈指针（刚进入异常时的栈指针，即用户栈指针） */
    LONG_S  t2, sp, PT_R3

    /* r4-r11：参数/返回值寄存器（系统调用的参数 1-8） */
    LONG_S  a0, sp, PT_R4           /* 参数 1 / 返回值 */
    LONG_S  a1, sp, PT_R5           /* 参数 2 */
    LONG_S  a2, sp, PT_R6           /* 参数 3 */
    LONG_S  a3, sp, PT_R7           /* 参数 4 */
    LONG_S  a4, sp, PT_R8           /* 参数 5 */
    LONG_S  a5, sp, PT_R9           /* 参数 6 */
    LONG_S  a6, sp, PT_R10          /* 参数 7 */
    LONG_S  a7, sp, PT_R11          /* 参数 8 / 系统调用号 */

    /*
     * r12-r20（t0-t8）：调用者保存寄存器
     * 这些寄存器不需要在此保存，因为它们是调用者保存的
     * 它们的值在栈中已由调用函数保存
     */

    /* r21：保留寄存器 u0 */
    LONG_S  u0, sp, PT_R21

    /* r22：栈帧指针/静态寄存器 fp */
    LONG_S  fp, sp, PT_R22

    /* r23-r31：静态寄存器（s0-s8） */
    SAVE_STATIC

    /*
     * 保存控制和状态寄存器（CSR）
     */

    /* ERA：异常返回地址寄存器（执行系统调用指令的地址） */
    csrrd   ra, LOONGARCH_CSR_ERA
    LONG_S  ra, sp, PT_ERA

    /* CRMD：当前模式信息寄存器 */
    csrrd   t2, LOONGARCH_CSR_CRMD
    LONG_S  t2, sp, PT_CRMD

    /* PRMD：异常前模式信息寄存器 */
    csrrd   t2, LOONGARCH_CSR_PRMD
    LONG_S  t2, sp, PT_PRMD

    /* EUEN：扩展部件使能寄存器 */
    csrrd   t2, LOONGARCH_CSR_EUEN
    LONG_S  t2, sp, PT_EUEN

    /* ECFG：异常配置寄存器 */
    csrrd   t2, LOONGARCH_CSR_ECFG
    LONG_S  t2, sp, PT_ECFG

    /* ESTAT：异常状态寄存器 */
    csrrd   t2, LOONGARCH_CSR_ESTAT
    LONG_S  t2, sp, PT_ESTAT

#ifdef CONFIG_KGDB
    /* 使能内核调试器 */
    li.w    t1, CSR_CRMD_WE
    csrxchg t1, t1, LOONGARCH_CSR_CRMD
#endif

    /*
     * 调用 C 语言系统调用处理函数
     *
     * a0 = sp：异常上下文指针
     * do_syscall 函数将根据系统调用号分发到具体的系统调用处理函数
     */
    move    a0, sp
    bl      do_syscall

    /*
     * 恢复上下文并返回用户空间
     *
     * RESTORE_ALL_AND_RET 宏将：
     * 1. 恢复所有寄存器
     * 2. 恢复 CSR 寄存器
     * 3. 执行 ertn 指令返回用户空间
     */
    RESTORE_ALL_AND_RET

ENDPROC(handle_syscall)
