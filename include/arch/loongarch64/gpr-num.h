/**
 * @file    gpr-num.h
 * @brief   LoongArch64通用寄存器编号定义
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件定义LoongArch64通用寄存器（GPR）的编号
 *          用于汇编代码中的寄存器编号映射
 *
 * @note MISRA-C:2012 合规
 * @note 此文件主要用于汇编代码
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef _GPR_NUM_H
#define _GPR_NUM_H

/************************宏 定 义******************************/

/**
 * @brief 定义通用寄存器编号
 *
 * @details 为汇编代码定义所有通用寄存器的符号编号
 *          - $r0-$r31: 编号0-31
 *          - $ra: 编号1
 *          - $tp: 编号2
 *          - $sp: 编号3
 *          - $a0-$a7: 编号4-11（参数寄存器）
 *          - $t0-$t8: 编号12-20（临时寄存器）
 *          - $fp: 编号22（帧指针）
 *          - $s0-$s8: 编号23-31（保存寄存器）
 */
#define __DEFINE_ASM_GPR_NUMS                                                                               \
    "    .equ    .L__gpr_num_zero, 0\n"                                                                     \
    "    .irp    num,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31\n" \
    "    .equ    .L__gpr_num_$r\\num, \\num\n"                                                              \
    "    .endr\n"                                                                                           \
    "    .equ    .L__gpr_num_$ra, 1\n"                                                                      \
    "    .equ    .L__gpr_num_$tp, 2\n"                                                                      \
    "    .equ    .L__gpr_num_$sp, 3\n"                                                                      \
    "    .irp    num,0,1,2,3,4,5,6,7\n"                                                                     \
    "    .equ    .L__gpr_num_$a\\num, 4 + \\num\n"                                                          \
    "    .endr\n"                                                                                           \
    "    .irp    num,0,1,2,3,4,5,6,7,8\n"                                                                   \
    "    .equ    .L__gpr_num_$t\\num, 12 + \\num\n"                                                         \
    "    .endr\n"                                                                                           \
    "    .equ    .L__gpr_num_$s9, 22\n"                                                                     \
    "    .equ    .L__gpr_num_$fp, 22\n"                                                                     \
    "    .irp    num,0,1,2,3,4,5,6,7,8\n"                                                                   \
    "    .equ    .L__gpr_num_$s\\num, 23 + \\num\n"                                                         \
    "    .endr\n"

#endif /* _GPR_NUM_H */
