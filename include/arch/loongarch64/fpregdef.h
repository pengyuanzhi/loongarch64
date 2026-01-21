/**
 * @file    fpregdef.h
 * @brief   LoongArch64浮点寄存器定义
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件定义LoongArch64浮点寄存器名称和常量
 *          - 浮点参数寄存器（fa0-fa7）
 *          - 浮点临时寄存器（ft0-ft15）
 *          - 浮点保存寄存器（fs0-fs7）
 *
 * @note MISRA-C:2012 合规
 * @note LoongArch64的FPU寄存器宽度为256位
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef _FPREGDEF_H
#define _FPREGDEF_H

/************************宏 定 义******************************/

/**
 * @defgroup FPFRegisters 浮点参数寄存器
 * @brief 浮点参数寄存器（用于函数参数传递）
 * @{
 */
#define fa0 $f0 /**< 浮点参数寄存器0 */
#define fa1 $f1 /**< 浮点参数寄存器1 */
#define fa2 $f2 /**< 浮点参数寄存器2 */
#define fa3 $f3 /**< 浮点参数寄存器3 */
#define fa4 $f4 /**< 浮点参数寄存器4 */
#define fa5 $f5 /**< 浮点参数寄存器5 */
#define fa6 $f6 /**< 浮点参数寄存器6 */
#define fa7 $f7 /**< 浮点参数寄存器7（fa0/fa1可复用为fv0/fv1返回值） */
/** @} */

/**
 * @defgroup FPTRegisters 浮点临时寄存器
 * @brief 浮点临时寄存器（调用者保存）
 * @{
 */
#define ft0 $f8   /**< 浮点临时寄存器0 */
#define ft1 $f9   /**< 浮点临时寄存器1 */
#define ft2 $f10  /**< 浮点临时寄存器2 */
#define ft3 $f11  /**< 浮点临时寄存器3 */
#define ft4 $f12  /**< 浮点临时寄存器4 */
#define ft5 $f13  /**< 浮点临时寄存器5 */
#define ft6 $f14  /**< 浮点临时寄存器6 */
#define ft7 $f15  /**< 浮点临时寄存器7 */
#define ft8 $f16  /**< 浮点临时寄存器8 */
#define ft9 $f17  /**< 浮点临时寄存器9 */
#define ft10 $f18 /**< 浮点临时寄存器10 */
#define ft11 $f19 /**< 浮点临时寄存器11 */
#define ft12 $f20 /**< 浮点临时寄存器12 */
#define ft13 $f21 /**< 浮点临时寄存器13 */
#define ft14 $f22 /**< 浮点临时寄存器14 */
#define ft15 $f23 /**< 浮点临时寄存器15 */
/** @} */

/**
 * @defgroup FPSRegisters 浮点保存寄存器
 * @brief 浮点保存寄存器（被调用者保存）
 * @{
 */
#define fs0 $f24 /**< 浮点保存寄存器0 */
#define fs1 $f25 /**< 浮点保存寄存器1 */
#define fs2 $f26 /**< 浮点保存寄存器2 */
#define fs3 $f27 /**< 浮点保存寄存器3 */
#define fs4 $f28 /**< 浮点保存寄存器4 */
#define fs5 $f29 /**< 浮点保存寄存器5 */
#define fs6 $f30 /**< 浮点保存寄存器6 */
#define fs7 $f31 /**< 浮点保存寄存器7 */
/** @} */

/**
 * @defgroup FPConstants FPU常量定义
 * @{
 */

/**
 * @brief FPU寄存器宽度（位）
 */
#define FPU_REG_WIDTH 256U

/**
 * @brief FPU数据对齐属性（32字节）
 */
#define FPU_ALIGN __attribute__((aligned(32)))

/** @} */

/************************C++兼容性******************************/
#ifdef __cplusplus
}
#endif

#endif /* _FPREGDEF_H */
