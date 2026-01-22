/**
 * @file    vector.S
 * @brief   LoongArch64 异常向量表和异常处理入口
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 本文件实现 LoongArch64 架构的异常向量表和异常处理入口
 *          - 异常向量表定义
 *          - 异常入口处理
 *          - 上下文保存和恢复
 *          - 异常分发到具体处理函数
 *
 * @note MISRA-C:2012 合规
 * @warning 异常处理必须快速完成，避免影响系统实时性
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
#include <asm.h>
#include <cpu.h>
#include <context.h>
#include <asm-offsets.h>
#include <system/const.h>

/*************************** 宏定义 ****************************/


/*************************** 类型定义 ****************************/


/*************************** 外部声明 ****************************/


/*************************** 前向声明 ****************************/


/*************************** 模块变量 ****************************/


/*************************** 函数实现 ****************************/

/**
 * @brief 异常向量表入口
 *
 * @details LoongArch64 异常向量表
 *          包含所有异常和中断的入口地址
 *
 * @note 向量表必须对齐到 512 字节边界
 * @note 每个异常占用 8 字节（一条指令）
 */
.section ".vectors", "ax"
.balign 512
.global vector_table
vector_table:

    /*
     * 异常向量 0-63：同步异常
     * 每个异常占用 8 字节
     */
    .rept 64
        b       exception_entry
    .endr

    /*
     * 异常向量 64-127：中断
     * 每个中断占用 8 字节
     */
    .rept 64
        b       interrupt_entry
    .endr

/**
 * @brief 异常入口处理
 *
 * @details 所有同步异常的统一入口
 *          执行流程：
 *          1. 保存通用寄存器到栈
 *          2. 保存特殊寄存器（CRMD、PRMD、EUEN、ECFG、ESTAT、ERA、BADV）
 *          3. 调用 C 语言异常处理函数
 *          4. 恢复寄存器
 *          5. 返回用户空间
 *
 * @note 此函数由硬件直接调用，必须保存完整上下文
 * @note 栈指针 sp 指向当前任务的异常栈
 */
.global exception_entry
exception_entry:
    /*
     * 分配栈空间保存上下文
     * 栈布局按照 pt_regs 结构体定义
     */
    addi.d  sp, sp, -PT_SIZE

    /*
     * 保存通用寄存器
     * 按照寄存器编号顺序保存
     */
    st.d    $r1, sp, PT_R1      /* ra */
    st.d    $r0, sp, PT_R2      /* tp */
    st.d    $sp, sp, PT_R3      /* sp (保存前的值) */
    st.d    $a0, sp, PT_R4
    st.d    $a1, sp, PT_R5
    st.d    $a2, sp, PT_R6
    st.d    $a3, sp, PT_R7
    st.d    $a4, sp, PT_R8
    st.d    $a5, sp, PT_R9
    st.d    $a6, sp, PT_R10
    st.d    $a7, sp, PT_R11
    st.d    $t0, sp, PT_R12
    st.d    $t1, sp, PT_R13
    st.d    $t2, sp, PT_R14
    st.d    $t3, sp, PT_R15
    st.d    $t4, sp, PT_R16
    st.d    $t5, sp, PT_R17
    st.d    $t6, sp, PT_R18
    st.d    $t7, sp, PT_R19
    st.d    $t8, sp, PT_R20
    st.d    $t9, sp, PT_R21
    st.d    $fp, sp, PT_R22
    st.d    $s0, sp, PT_R23
    st.d    $s1, sp, PT_R24
    st.d    $s2, sp, PT_R25
    st.d    $s3, sp, PT_R26
    st.d    $s4, sp, PT_R27
    st.d    $s5, sp, PT_R28
    st.d    $s6, sp, PT_R29
    st.d    $s7, sp, PT_R30
    st.d    $s8, sp, PT_R31

    /*
     * 保存特殊寄存器
     */
    csrrd   $t0, LOONGARCH_CSR_CRMD
    st.d    $t0, sp, PT_CRMD

    csrrd   $t0, LOONGARCH_CSR_PRMD
    st.d    $t0, sp, PT_PRMD

    csrrd   $t0, LOONGARCH_CSR_EUEN
    st.d    $t0, sp, PT_EUEN

    csrrd   $t0, LOONGARCH_CSR_ECFG
    st.d    $t0, sp, PT_ECFG

    csrrd   $t0, LOONGARCH_CSR_ESTAT
    st.d    $t0, sp, PT_ESTAT

    csrrd   $t0, LOONGARCH_CSR_ERA
    st.d    $t0, sp, PT_ERA

    csrrd   $t0, LOONGARCH_CSR_BADV
    st.d    $t0, sp, PT_BVADDR

    /*
     * 调用 C 语言异常处理函数
     * 参数 a0 = pt_regs 指针
     */
    move    $a0, sp
    bl      do_exception

    /*
     * 恢复上下文
     * 从 do_exception 返回后，恢复所有寄存器
     */
    ld.d    $t0, sp, PT_CRMD
    csrwr   $t0, LOONGARCH_CSR_CRMD

    ld.d    $t0, sp, PT_PRMD
    csrwr   $t0, LOONGARCH_CSR_PRMD

    ld.d    $t0, sp, PT_EUEN
    csrwr   $t0, LOONGARCH_CSR_EUEN

    ld.d    $t0, sp, PT_ECFG
    csrwr   $t0, LOONGARCH_CSR_ECFG

    ld.d    $t0, sp, PT_ESTAT
    csrwr   $t0, LOONGARCH_CSR_ESTAT

    ld.d    $t0, sp, PT_ERA
    csrwr   $t0, LOONGARCH_CSR_ERA

    ld.d    $t0, sp, PT_BVADDR
    csrwr   $t0, LOONGARCH_CSR_BADV

    ld.d    $ra, sp, PT_R1
    ld.d    $tp, sp, PT_R2
    ld.d    $a0, sp, PT_R4
    ld.d    $a1, sp, PT_R5
    ld.d    $a2, sp, PT_R6
    ld.d    $a3, sp, PT_R7
    ld.d    $a4, sp, PT_R8
    ld.d    $a5, sp, PT_R9
    ld.d    $a6, sp, PT_R10
    ld.d    $a7, sp, PT_R11
    ld.d    $t0, sp, PT_R12
    ld.d    $t1, sp, PT_R13
    ld.d    $t2, sp, PT_R14
    ld.d    $t3, sp, PT_R15
    ld.d    $t4, sp, PT_R16
    ld.d    $t5, sp, PT_R17
    ld.d    $t6, sp, PT_R18
    ld.d    $t7, sp, PT_R19
    ld.d    $t8, sp, PT_R20
    ld.d    $t9, sp, PT_R21
    ld.d    $fp, sp, PT_R22
    ld.d    $s0, sp, PT_R23
    ld.d    $s1, sp, PT_R24
    ld.d    $s2, sp, PT_R25
    ld.d    $s3, sp, PT_R26
    ld.d    $s4, sp, PT_R27
    ld.d    $s5, sp, PT_R28
    ld.d    $s6, sp, PT_R29
    ld.d    $s7, sp, PT_R30
    ld.d    $s8, sp, PT_R31

    /*
     * 恢复栈指针并返回
     */
    addi.d  sp, sp, PT_SIZE
    ertn

/**
 * @brief 中断入口处理
 *
 * @details 所有中断的统一入口
 *          执行流程：
 *          1. 保存通用寄存器到栈
 *          2. 保存特殊寄存器
 *          3. 调用 C 语言中断处理函数
 *          4. 恢复寄存器
 *          5. 返回被中断的代码
 *
 * @note 此函数由硬件直接调用，必须保存完整上下文
 */
.global interrupt_entry
interrupt_entry:
    /*
     * 分配栈空间保存上下文
     */
    addi.d  sp, sp, -PT_SIZE

    /*
     * 保存通用寄存器
     */
    st.d    $r1, sp, PT_R1
    st.d    $r0, sp, PT_R2
    st.d    $sp, sp, PT_R3
    st.d    $a0, sp, PT_R4
    st.d    $a1, sp, PT_R5
    st.d    $a2, sp, PT_R6
    st.d    $a3, sp, PT_R7
    st.d    $a4, sp, PT_R8
    st.d    $a5, sp, PT_R9
    st.d    $a6, sp, PT_R10
    st.d    $a7, sp, PT_R11
    st.d    $t0, sp, PT_R12
    st.d    $t1, sp, PT_R13
    st.d    $t2, sp, PT_R14
    st.d    $t3, sp, PT_R15
    st.d    $t4, sp, PT_R16
    st.d    $t5, sp, PT_R17
    st.d    $t6, sp, PT_R18
    st.d    $t7, sp, PT_R19
    st.d    $t8, sp, PT_R20
    st.d    $t9, sp, PT_R21
    st.d    $fp, sp, PT_R22
    st.d    $s0, sp, PT_R23
    st.d    $s1, sp, PT_R24
    st.d    $s2, sp, PT_R25
    st.d    $s3, sp, PT_R26
    st.d    $s4, sp, PT_R27
    st.d    $s5, sp, PT_R28
    st.d    $s6, sp, PT_R29
    st.d    $s7, sp, PT_R30
    st.d    $s8, sp, PT_R31

    /*
     * 保存特殊寄存器
     */
    csrrd   $t0, LOONGARCH_CSR_CRMD
    st.d    $t0, sp, PT_CRMD

    csrrd   $t0, LOONGARCH_CSR_PRMD
    st.d    $t0, sp, PT_PRMD

    csrrd   $t0, LOONGARCH_CSR_EUEN
    st.d    $t0, sp, PT_EUEN

    csrrd   $t0, LOONGARCH_CSR_ECFG
    st.d    $t0, sp, PT_ECFG

    csrrd   $t0, LOONGARCH_CSR_ESTAT
    st.d    $t0, sp, PT_ESTAT

    csrrd   $t0, LOONGARCH_CSR_ERA
    st.d    $t0, sp, PT_ERA

    csrrd   $t0, LOONGARCH_CSR_BADV
    st.d    $t0, sp, PT_BVADDR

    /*
     * 调用 C 语言中断处理函数
     */
    move    $a0, sp
    bl      do_interrupt

    /*
     * 恢复上下文
     */
    ld.d    $t0, sp, PT_CRMD
    csrwr   $t0, LOONGARCH_CSR_CRMD

    ld.d    $t0, sp, PT_PRMD
    csrwr   $t0, LOONGARCH_CSR_PRMD

    ld.d    $t0, sp, PT_EUEN
    csrwr   $t0, LOONGARCH_CSR_EUEN

    ld.d    $t0, sp, PT_ECFG
    csrwr   $t0, LOONGARCH_CSR_ECFG

    ld.d    $t0, sp, PT_ESTAT
    csrwr   $t0, LOONGARCH_CSR_ESTAT

    ld.d    $t0, sp, PT_ERA
    csrwr   $t0, LOONGARCH_CSR_ERA

    ld.d    $t0, sp, PT_BVADDR
    csrwr   $t0, LOONGARCH_CSR_BADV

    ld.d    $ra, sp, PT_R1
    ld.d    $tp, sp, PT_R2
    ld.d    $a0, sp, PT_R4
    ld.d    $a1, sp, PT_R5
    ld.d    $a2, sp, PT_R6
    ld.d    $a3, sp, PT_R7
    ld.d    $a4, sp, PT_R8
    ld.d    $a5, sp, PT_R9
    ld.d    $a6, sp, PT_R10
    ld.d    $a7, sp, PT_R11
    ld.d    $t0, sp, PT_R12
    ld.d    $t1, sp, PT_R13
    ld.d    $t2, sp, PT_R14
    ld.d    $t3, sp, PT_R15
    ld.d    $t4, sp, PT_R16
    ld.d    $t5, sp, PT_R17
    ld.d    $t6, sp, PT_R18
    ld.d    $t7, sp, PT_R19
    ld.d    $t8, sp, PT_R20
    ld.d    $t9, sp, PT_R21
    ld.d    $fp, sp, PT_R22
    ld.d    $s0, sp, PT_R23
    ld.d    $s1, sp, PT_R24
    ld.d    $s2, sp, PT_R25
    ld.d    $s3, sp, PT_R26
    ld.d    $s4, sp, PT_R27
    ld.d    $s5, sp, PT_R28
    ld.d    $s6, sp, PT_R29
    ld.d    $s7, sp, PT_R30
    ld.d    $s8, sp, PT_R31

    /*
     * 恢复栈指针并返回
     */
    addi.d  sp, sp, PT_SIZE
    ertn
