/**
 * @file    asmLoongarch.h
 * @brief   LoongArch汇编寄存器定义
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 本文件定义LoongArch架构寄存器名称别名
 *          - 通用寄存器（GPR）定义
 *          - 浮点寄存器（FPR）定义
 *          - 浮点控制状态寄存器定义
 *          - 浮点条件码寄存器定义
 *          - 寄存器大小常量
 *
 * @note MISRA-C:2012 合规
 * @note 本文件主要用于汇编代码
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef __TTOS_ASMLOONGARCH_H__
#define __TTOS_ASMLOONGARCH_H__

/************************头 文 件******************************/
/************************宏 定 义******************************/

/**
 * @defgroup GPRRegisters 通用寄存器定义
 * @brief LoongArch通用寄存器（$r0-$r31）
 * @{
 */

/**
 * @defgroup GPRSpecial 特殊通用寄存器
 * @{
 */
#define zero $r0 /**< 零寄存器（硬连线为0） */
#define ra $r1 /**< 返回地址寄存器 */
#define tp $r2 /**< 线程指针寄存器 */
#define gp $r2 /**< 全局指针寄存器（别名） */
#define sp $r3 /**< 栈指针寄存器 */
/** @} */

/**
 * @defgroup GPRValue 返回值寄存器
 * @{
 */
#define v0 $r4 /**< 返回值寄存器0（调用者保存） */
#define v1 $r5 /**< 返回值寄存器1（调用者保存） */
/** @} */

/**
 * @defgroup GPRArgument 参数寄存器
 * @{
 */
#define a0 $r4 /**< 参数寄存器0（复用v0） */
#define a1 $r5 /**< 参数寄存器1（复用v1） */
#define a2 $r6 /**< 参数寄存器2 */
#define a3 $r7 /**< 参数寄存器3 */
#define a4 $r8 /**< 参数寄存器4 */
#define a5 $r9 /**< 参数寄存器5 */
#define a6 $r10 /**< 参数寄存器6 */
#define a7 $r11 /**< 参数寄存器7 */
/** @} */

/**
 * @defgroup GPRTemporary 临时寄存器
 * @brief 临时寄存器（调用者保存，函数可自由使用）
 * @{
 */
#define t0 $r12 /**< 临时寄存器0 */
#define t1 $r13 /**< 临时寄存器1 */
#define t2 $r14 /**< 临时寄存器2 */
#define t3 $r15 /**< 临时寄存器3 */
#define t4 $r16 /**< 临时寄存器4 */
#define t5 $r17 /**< 临时寄存器5 */
#define t6 $r18 /**< 临时寄存器6 */
#define t7 $r19 /**< 临时寄存器7 */
#define t8 $r20 /**< 临时寄存器8 */
/** @} */

/**
 * @defgroup GPRReserved 保留寄存器
 * @{
 */
#define AT $r21 /**< 汇编器临时寄存器 */
/** @} */

/**
 * @defgroup GPRFrame 帧指针和保存寄存器
 * @{
 */
#define fp $r22 /**< 帧指针寄存器 */
#define s0 $r23 /**< 保存寄存器0（被调用者保存） */
#define s1 $r24 /**< 保存寄存器1（被调用者保存） */
#define s2 $r25 /**< 保存寄存器2（被调用者保存） */
#define s3 $r26 /**< 保存寄存器3（被调用者保存） */
#define s4 $r27 /**< 保存寄存器4（被调用者保存） */
#define s5 $r28 /**< 保存寄存器5（被调用者保存） */
#define s6 $r29 /**< 保存寄存器6（被调用者保存） */
#define s7 $r30 /**< 保存寄存器7（被调用者保存） */
#define s8 $r31 /**< 保存寄存器8（被调用者保存） */
/** @} */

/** @} */

/**
 * @defgroup FPRRegisters 浮点寄存器定义
 * @brief LoongArch浮点寄存器（$f0-$f31）
 * @{
 */

/**
 * @defgroup FPRReturn 浮点返回值寄存器
 * @{
 */
#define fp0 $f0 /**< 浮点返回值寄存器0 */
#define fp1 $f1 /**< 浮点返回值寄存器1 */
#define fp2 $f2 /**< 浮点返回值寄存器2 */
#define fp3 $f3 /**< 浮点返回值寄存器3 */
/** @} */

/**
 * @defgroup FPRTemporary 浮点调用者保存寄存器
 * @{
 */
#define fp4 $f4 /**< 浮点调用者保存寄存器0 */
#define fp5 $f5 /**< 浮点调用者保存寄存器1 */
#define fp6 $f6 /**< 浮点调用者保存寄存器2 */
#define fp7 $f7 /**< 浮点调用者保存寄存器3 */
#define fp8 $f8 /**< 浮点调用者保存寄存器4 */
#define fp9 $f9 /**< 浮点调用者保存寄存器5 */
#define fp10 $f10 /**< 浮点调用者保存寄存器6 */
#define fp11 $f11 /**< 浮点调用者保存寄存器7 */
/** @} */

/**
 * @defgroup FPRArgument 浮点参数寄存器
 * @{
 */
#define fp12 $f12 /**< 浮点参数寄存器0 */
#define fp13 $f13 /**< 浮点参数寄存器1 */
#define fp14 $f14 /**< 浮点参数寄存器2 */
#define fp15 $f15 /**< 浮点参数寄存器3 */
/** @} */

/**
 * @defgroup FPRTemporary2 浮点调用者保存寄存器（第二组）
 * @{
 */
#define fp16 $f16 /**< 浮点调用者保存寄存器8 */
#define fp17 $f17 /**< 浮点调用者保存寄存器9 */
#define fp18 $f18 /**< 浮点调用者保存寄存器10 */
#define fp19 $f19 /**< 浮点调用者保存寄存器11 */
/** @} */

/**
 * @defgroup FPRCalleeSaved 浮点被调用者保存寄存器
 * @{
 */
#define fp20 $f20 /**< 浮点被调用者保存寄存器0 */
#define fp21 $f21 /**< 浮点被调用者保存寄存器1 */
#define fp22 $f22 /**< 浮点被调用者保存寄存器2 */
#define fp23 $f23 /**< 浮点被调用者保存寄存器3 */
#define fp24 $f24 /**< 浮点被调用者保存寄存器4 */
#define fp25 $f25 /**< 浮点被调用者保存寄存器5 */
#define fp26 $f26 /**< 浮点被调用者保存寄存器6 */
#define fp27 $f27 /**< 浮点被调用者保存寄存器7 */
#define fp28 $f28 /**< 浮点被调用者保存寄存器8 */
#define fp29 $f29 /**< 浮点被调用者保存寄存器9 */
#define fp30 $f30 /**< 浮点被调用者保存寄存器10 */
#define fp31 $f31 /**< 浮点被调用者保存寄存器11 */
/** @} */

/** @} */

/**
 * @defgroup FCSRRegisters 浮点控制状态寄存器
 * @{
 */
#define fcsr0 $r0 /**< 浮点控制状态寄存器0 */
#define fcsr1 $r1 /**< 浮点控制状态寄存器1 */
#define fcsr2 $r2 /**< 浮点控制状态寄存器2 */
#define fcsr3 $r3 /**< 浮点控制状态寄存器3 */
/** @} */

/**
 * @defgroup FCCRegisters 浮点条件码寄存器
 * @{
 */
#define fcc0 $fcc0 /**< 浮点条件码0 */
#define fcc1 $fcc1 /**< 浮点条件码1 */
#define fcc2 $fcc2 /**< 浮点条件码2 */
#define fcc3 $fcc3 /**< 浮点条件码3 */
#define fcc4 $fcc4 /**< 浮点条件码4 */
#define fcc5 $fcc5 /**< 浮点条件码5 */
#define fcc6 $fcc6 /**< 浮点条件码6 */
#define fcc7 $fcc7 /**< 浮点条件码7 */
/** @} */

/**
 * @defgroup RegisterSize 寄存器大小常量
 * @{
 */
#ifndef __loongarch64
#define SZREG 4U /**< 32位架构寄存器宽度（字节） */
#else
#define SZREG 8U /**< 64位架构寄存器宽度（字节） */
#endif
#define LONGSIZE 8U /**< long类型大小（字节） */
/** @} */

/************************类型定义******************************/
/************************外部变量******************************/
/************************外部函数******************************/
/************************内联函数******************************/
/************************C++兼容性******************************/
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif /* __TTOS_ASMLOONGARCH_H__ */
