/**
 * @file    asm.h
 * @brief   LoongArch汇编实用宏定义
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 本文件定义LoongArch汇编代码中使用的实用宏
 *          - 寄存器别名定义（通用寄存器、浮点寄存器）
 *          - 加载/存储指令宏（根据32/64位自动适配）
 *          - 栈对齐和预取指令宏
 *          - 异常处理宏（保存/恢复上下文）
 *          - FPU状态保存/恢复宏
 *          - 任务上下文切换宏
 *
 * @note MISRA-C:2012 合规
 * @note 本文件主要用于汇编代码和汇编器
 *
 * @warning 宏定义必须与汇编代码中的偏移量一致
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef _ASM_H
#define _ASM_H

/************************头 文 件******************************/
#include <asm-offsets.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*************************** 寄存器定义 ****************************/

/**
 * @defgroup GPRAliases 通用寄存器别名
 * @brief LoongArch通用寄存器（$r0-$r31）别名定义
 * @{
 */

/**
 * @defgroup GPRSpecial 特殊寄存器
 * @{
 */
#define zero $r0 /**< 零寄存器（硬连线为0） */
#define ra $r1   /**< 返回地址寄存器 */
#define tp $r2   /**< 线程指针寄存器 */
#define gp $r2   /**< 全局指针寄存器（别名） */
#define sp $r3   /**< 栈指针寄存器 */
/** @} */

/**
 * @defgroup GPRValue 返回值和参数寄存器
 * @{
 */
#define v0 $r4 /**< 返回值寄存器0（调用者保存） */
#define v1 $r5 /**< 返回值寄存器1（调用者保存） */
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
#define u0 $r21 /**< 汇编器临时寄存器 */
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
 * @defgroup FPRAliases 浮点寄存器别名
 * @brief LoongArch浮点寄存器（$f0-$f31）别名定义
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
 * @defgroup FPRTemporary 浮点调用者保存寄存器（第一组）
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

/*************************** 寄存器大小 ***************************/

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

/*************************** 预取指令 ****************************/

/**
 * @defgroup PrefetchMacros 预取指令宏
 * @{
 */

#ifdef CONFIG_CPU_HAS_PREFETCH
#define PREF(hint, addr, offs) preld hint, addr, offs /**< 数据预取 */
#define PREFX(hint, addr, index) preldx hint, addr, index /**< 变址预取 */
#else
#define PREF(hint, addr, offs) /**< 预取（空操作） */
#define PREFX(hint, addr, index) /**< 变址预取（空操作） */
#endif

/** @} */

/*************************** 栈对齐 ******************************/

/**
 * @defgroup StackAlignment 栈对齐
 * @{
 */
#define STACK_ALIGN ~(0xf) /**< 16字节栈对齐掩码 */
/** @} */

/*************************** 加载/存储指令 ************************/

/**
 * @defgroup LoadStoreMacros 加载/存储指令宏
 * @brief 根据32/64位自动适配加载/存储指令
 * @{
 */

#if (SZREG == 4U)
#define REG_L ld.w /**< 寄存器加载（32位） */
#define REG_S st.w /**< 寄存器存储（32位） */
#define REG_ADD add.w /**< 寄存器加法（32位） */
#define REG_SUB sub.w /**< 寄存器减法（32位） */
#else
#define REG_L ld.d /**< 寄存器加载（64位） */
#define REG_S st.d /**< 寄存器存储（64位） */
#define REG_ADD add.d /**< 寄存器加法（64位） */
#define REG_SUB sub.d /**< 寄存器减法（64位） */
#endif

/** @} */

/**
 * @defgroup IntMacros int类型操作宏
 * @{
 */
#if (__SIZEOF_INT__ == 4)
#define INT_ADD add.w /**< int加法 */
#define INT_ADDI addi.w /**< int立即数加法 */
#define INT_SUB sub.w /**< int减法 */
#define INT_L ld.w /**< int加载 */
#define INT_S st.w /**< int存储 */
#define INT_SLL slli.w /**< int逻辑左移（立即数） */
#define INT_SLLV sll.w /**< int逻辑左移（变量） */
#define INT_SRL srli.w /**< int逻辑右移（立即数） */
#define INT_SRLV srl.w /**< int逻辑右移（变量） */
#define INT_SRA srai.w /**< int算术右移（立即数） */
#define INT_SRAV sra.w /**< int算术右移（变量） */
#endif

#if (__SIZEOF_INT__ == 8)
#define INT_ADD add.d /**< int加法（64位） */
#define INT_ADDI addi.d /**< int立即数加法（64位） */
#define INT_SUB sub.d /**< int减法（64位） */
#define INT_L ld.d /**< int加载（64位） */
#define INT_S st.d /**< int存储（64位） */
#define INT_SLL slli.d /**< int逻辑左移（64位立即数） */
#define INT_SLLV sll.d /**< int逻辑左移（64位变量） */
#define INT_SRL srli.d /**< int逻辑右移（64位立即数） */
#define INT_SRLV srl.d /**< int逻辑右移（64位变量） */
#define INT_SRA srai.d /**< int算术右移（64位立即数） */
#define INT_SRAV sra.d /**< int算术右移（64位变量） */
#endif

/** @} */

/**
 * @defgroup LongMacros long类型操作宏
 * @{
 */
#if (__SIZEOF_LONG__ == 4)
#define LONG_ADD add.w /**< long加法（32位） */
#define LONG_ADDI addi.w /**< long立即数加法（32位） */
#define LONG_SUB sub.w /**< long减法（32位） */
#define LONG_L ld.w /**< long加载（32位） */
#define LONG_S st.w /**< long存储（32位） */
#define LONG_SLL slli.w /**< long逻辑左移（32位立即数） */
#define LONG_SLLV sll.w /**< long逻辑左移（32位变量） */
#define LONG_SRL srli.w /**< long逻辑右移（32位立即数） */
#define LONG_SRLV srl.w /**< long逻辑右移（32位变量） */
#define LONG_SRA srai.w /**< long算术右移（32位立即数） */
#define LONG_SRAV sra.w /**< long算术右移（32位变量） */
#ifdef ASM_USE
#define LONG .word /**< long汇编定义（32位） */
#endif
#define LONGSIZE 4U /**< long大小（32位） */
#define LONGMASK 3U /**< long掩码（32位） */
#define LONGLOG 2U /**< long对数（32位） */
#endif

#if (__SIZEOF_LONG__ == 8)
#define LONG_ADD add.d /**< long加法（64位） */
#define LONG_ADDI addi.d /**< long立即数加法（64位） */
#define LONG_SUB sub.d /**< long减法（64位） */
#define LONG_L ld.d /**< long加载（64位） */
#define LONG_S st.d /**< long存储（64位） */
#define LONG_SLL slli.d /**< long逻辑左移（64位立即数） */
#define LONG_SLLV sll.d /**< long逻辑左移（64位变量） */
#define LONG_SRL srli.d /**< long逻辑右移（64位立即数） */
#define LONG_SRLV srl.d /**< long逻辑右移（64位变量） */
#define LONG_SRA srai.d /**< long算术右移（64位立即数） */
#define LONG_SRAV sra.d /**< long算术右移（64位变量） */
#ifdef ASM_USE
#define LONG .dword /**< long汇编定义（64位） */
#endif
#define LONGSIZE 8U /**< long大小（64位） */
#define LONGMASK 7U /**< long掩码（64位） */
#define LONGLOG 3U /**< long对数（64位） */
#endif

/** @} */

/**
 * @defgroup PointerMacros 指针操作宏
 * @{
 */
#if (__SIZEOF_POINTER__ == 4)
#define PTR_ADD add.w /**< 指针加法（32位） */
#define PTR_ADDI addi.w /**< 指针立即数加法（32位） */
#define PTR_SUB sub.w /**< 指针减法（32位） */
#define PTR_L ld.w /**< 指针加载（32位） */
#define PTR_S st.w /**< 指针存储（32位） */
#define PTR_LI li.w /**< 指针立即数加载（32位） */
#define PTR_SLL slli.w /**< 指针逻辑左移（32位立即数） */
#define PTR_SLLV sll.w /**< 指针逻辑左移（32位变量） */
#define PTR_SRL srli.w /**< 指针逻辑右移（32位立即数） */
#define PTR_SRLV srl.w /**< 指针逻辑右移（32位变量） */
#define PTR_SRA srai.w /**< 指针算术右移（32位立即数） */
#define PTR_SRAV sra.w /**< 指针算术右移（32位变量） */
#define PTR_SCALESHIFT 2U /**< 指针缩放位移（32位） */
#ifdef ASM_USE
#define PTR .word /**< 指针汇编定义（32位） */
#endif
#define PTRSIZE 4U /**< 指针大小（32位） */
#define PTRLOG 2U /**< 指针对数（32位） */
#endif

#if (__SIZEOF_POINTER__ == 8)
#define PTR_ADD add.d /**< 指针加法（64位） */
#define PTR_ADDI addi.d /**< 指针立即数加法（64位） */
#define PTR_SUB sub.d /**< 指针减法（64位） */
#define PTR_L ld.d /**< 指针加载（64位） */
#define PTR_S st.d /**< 指针存储（64位） */
#define PTR_LI li.d /**< 指针立即数加载（64位） */
#define PTR_SLL slli.d /**< 指针逻辑左移（64位立即数） */
#define PTR_SLLV sll.d /**< 指针逻辑左移（64位变量） */
#define PTR_SRL srli.d /**< 指针逻辑右移（64位立即数） */
#define PTR_SRLV srl.d /**< 指针逻辑右移（64位变量） */
#define PTR_SRA srai.d /**< 指针算术右移（64位立即数） */
#define PTR_SRAV sra.d /**< 指针算术右移（64位变量） */
#define PTR_SCALESHIFT 3U /**< 指针缩放位移（64位） */
#ifdef ASM_USE
#define PTR .dword /**< 指针汇编定义（64位） */
#endif
#define PTRSIZE 8U /**< 指针大小（64位） */
#define PTRLOG 3U /**< 指针对数（64位） */
#endif

/** @} */

/*************************** Kprobes 注解 ************************/

/**
 * @defgroup KprobesMacros Kprobes相关宏
 * @{
 */

#ifdef CONFIG_KPROBES
#define _ASM_NOKPROBE(name) \
    .pushsection "_kprobe_blacklist", "aw"; \
    .quad name; \
    .popsection
#else
#define _ASM_NOKPROBE(name)
#endif

/** @} */

/*************************** 汇编实用宏 *************************/

#ifdef ASM_USE

/**
 * @defgroup AsmUtils 汇编实用宏
 * @{
 */

#define ALIGN .align 4 /**< 对齐宏 */
#define ENTRY(name) \
    .globl name; \
    ALIGN; \
    name: /**< 函数入口宏 */
#define GLOBAL(name) \
    .globl name; \
    name: /**< 全局符号宏 */
#define END(name) .size name, .-name /**< 函数结束宏 */
#define ENDPROC(name) \
    .type name, % function; \
    END(name) /**< 函数过程结束宏 */

/**
 * @brief 备份t0和t1寄存器到内核态Scratch寄存器
 */
.macro BACKUP_T0T1
    csrwr t0, EXCEPTION_KS0
    csrwr t1, EXCEPTION_KS1
.endm

/**
 * @brief 从内核态Scratch寄存器恢复t0和t1寄存器
 */
.macro RELOAD_T0T1
    csrrd t0, EXCEPTION_KS0
    csrrd t1, EXCEPTION_KS1
.endm

/**
 * @brief 获取保存的栈指针
 * @details 在多核环境下根据CPU ID获取对应的内核栈
 */
.macro get_saved_sp
#ifdef CONFIG_SMP
    csrrd t0, LOONGARCH_CSR_CPUID
    li.w t1, __SIZEOF_POINTER__
    mul.w t0, t0, t1
#endif
    la_abs t1, running_task_stack
#ifdef CONFIG_SMP
    LONG_ADD t1, t1, t0
#endif
    move t0, sp
    LONG_L sp, t1, 0
.endm

/**
 * @brief 设置保存的栈指针
 * @param stackp 栈指针
 * @param temp 临时寄存器
 * @param temp2 第二个临时寄存器（未使用）
 */
.macro set_saved_sp stackp, temp, temp2
    la.pcrel \temp, kernelsp
#ifdef CONFIG_SMP
    LONG_ADD \temp, \temp, u0
#endif
    LONG_S \stackp, \temp, 0
.endm

/**
 * @brief 保存部分寄存器到栈
 * @details 保存异常/中断发生时的关键寄存器
 */
.macro SAVE_SOME
    csrrd t1, LOONGARCH_CSR_PRMD
    andi t1, t1, 0x3
    move t0, sp
    beqz t1, 0f
    get_saved_sp
0:
    PTR_ADDI sp, sp, -PT_SIZE
    LONG_S zero, sp, PT_R0
    LONG_S ra, sp, PT_R1
    LONG_S tp, sp, PT_R2
    LONG_S t0, sp, PT_R3
    LONG_S a0, sp, PT_R4
    LONG_S a1, sp, PT_R5
    LONG_S a2, sp, PT_R6
    LONG_S a3, sp, PT_R7
    LONG_S a4, sp, PT_R8
    LONG_S a5, sp, PT_R9
    LONG_S a6, sp, PT_R10
    LONG_S a7, sp, PT_R11
    LONG_S fp, sp, PT_R22
    csrrd ra, LOONGARCH_CSR_ERA
    LONG_S ra, sp, PT_ERA
    csrrd t0, LOONGARCH_CSR_PRMD
    LONG_S t0, sp, PT_PRMD
    csrrd t0, LOONGARCH_CSR_CRMD
    LONG_S t0, sp, PT_CRMD
    csrrd t0, LOONGARCH_CSR_EUEN
    LONG_S t0, PT_EUEN
    csrrd t0, LOONGARCH_CSR_ECFG
    LONG_S t0, PT_ECFG
    csrrd t0, LOONGARCH_CSR_ESTAT
    PTR_S t0, sp, PT_ESTAT
    csrrd t0, LOONGARCH_CSR_PRMD
    andi t0, t0, 0x3
    beqz t0, 1f
    LONG_S u0, sp, PT_R21
1:
.endm

/**
 * @brief 保存临时寄存器到栈
 */
.macro SAVE_TEMP
    LONG_S t0, sp, PT_R12
    LONG_S t1, sp, PT_R13
    LONG_S t2, sp, PT_R14
    LONG_S t3, sp, PT_R15
    LONG_S t4, sp, PT_R16
    LONG_S t5, sp, PT_R17
    LONG_S t6, sp, PT_R18
    LONG_S t7, sp, PT_R19
    LONG_S t8, sp, PT_R20
.endm

/**
 * @brief 保存静态寄存器到栈
 */
.macro SAVE_STATIC
    LONG_S s0, sp, PT_R23
    LONG_S s1, sp, PT_R24
    LONG_S s2, sp, PT_R25
    LONG_S s3, sp, PT_R26
    LONG_S s4, sp, PT_R27
    LONG_S s5, sp, PT_R28
    LONG_S s6, sp, PT_R29
    LONG_S s7, sp, PT_R30
    LONG_S s8, sp, PT_R31
.endm

/**
 * @brief 保存全部寄存器到栈
 */
.macro SAVE_ALL
    BACKUP_T0T1
    SAVE_SOME
    RELOAD_T0T1
    SAVE_TEMP
    SAVE_STATIC
.endm

/**
 * @brief 从栈恢复临时寄存器
 */
.macro RESTORE_TEMP
    LONG_L t0, sp, PT_R12
    LONG_L t1, sp, PT_R13
    LONG_L t2, sp, PT_R14
    LONG_L t3, sp, PT_R15
    LONG_L t4, sp, PT_R16
    LONG_L t5, sp, PT_R17
    LONG_L t6, sp, PT_R18
    LONG_L t7, sp, PT_R19
    LONG_L t8, sp, PT_R20
.endm

/**
 * @brief 从栈恢复静态寄存器
 */
.macro RESTORE_STATIC
    LONG_L s0, sp, PT_R23
    LONG_L s1, sp, PT_R24
    LONG_L s2, sp, PT_R25
    LONG_L s3, sp, PT_R26
    LONG_L s4, sp, PT_R27
    LONG_L s5, sp, PT_R28
    LONG_L s6, sp, PT_R29
    LONG_L s7, sp, PT_R30
    LONG_L s8, sp, PT_R31
.endm

/**
 * @brief 从栈恢复部分寄存器
 */
.macro RESTORE_SOME
    LONG_L a0, sp, PT_ERA
    csrwr a0, LOONGARCH_CSR_ERA
    LONG_L a0, sp, PT_PRMD
    csrwr a0, LOONGARCH_CSR_PRMD
    andi a0, a0, 0x3
    beqz a0, 0f
    LONG_L u0, sp, PT_R21
0:
    LONG_L ra, sp, PT_R1
    LONG_L tp, sp, PT_R2
    LONG_L a0, sp, PT_R4
    LONG_L a1, sp, PT_R5
    LONG_L a2, sp, PT_R6
    LONG_L a3, sp, PT_R7
    LONG_L a4, sp, PT_R8
    LONG_L a5, sp, PT_R9
    LONG_L a6, sp, PT_R10
    LONG_L a7, sp, PT_R11
    LONG_L fp, sp, PT_R22
.endm

/**
 * @brief 恢复栈指针并返回
 */
.macro RESTORE_SP_AND_RET
    LONG_L sp, sp, PT_R3
    ertn
.endm

/**
 * @brief 恢复全部寄存器并返回
 */
.macro RESTORE_ALL_AND_RET
    RESTORE_STATIC
    RESTORE_TEMP
    RESTORE_SOME
    RESTORE_SP_AND_RET
.endm

/**
 * @brief 加载绝对地址
 * @param reg 目标寄存器
 * @param sym 符号名称
 */
.macro la_abs reg, sym
#ifndef CONFIG_RELOCATABLE
    la.abs \reg, \sym
#else
766:
    lu12i.w \reg, 0
    ori \reg, \reg, 0
    lu32i.d \reg, 0
    lu52i.d \reg, \reg, 0
    .pushsection ".la_abs", "aw", %progbits
    .p2align 3
    .dword 766b
    .dword \sym
    .popsection
#endif
.endm

/**
 * @brief 异常表条目宏
 * @param insn 可能出错的指令
 * @param fixup 异常处理跳转地址
 */
.macro _asm_extable, insn, fixup
    __ASM_EXTABLE_RAW(\insn, \fixup, EX_TYPE_FIXUP, 0)
.endm

/** @} */

/*************************** FPU保存/恢复 ************************/

/**
 * @defgroup FPUMacros FPU保存/恢复宏
 * @{
 */

/**
 * @brief 保存FPU控制状态寄存器
 * @param thread 线程结构体指针
 * @param tmp 临时寄存器
 */
.macro fpu_save_csr thread, tmp
    movfcsr2gr \tmp, fcsr0
    st.d \tmp, \thread, THREAD_FCSR
#ifdef CONFIG_CPU_HAS_LBT
    andi \tmp, \tmp, FPU_CSR_TM
    beqz \tmp, 1f
    x86mftop \tmp
    st.w \tmp, \thread, THREAD_FTOP
    x86clrtm
1:
#endif
.endm

/**
 * @brief 恢复FPU控制状态寄存器
 * @param thread 线程结构体指针
 * @param tmp0 临时寄存器0
 * @param tmp1 临时寄存器1
 */
.macro fpu_restore_csr thread, tmp0, tmp1
    ld.w \tmp0, \thread, THREAD_FCSR
    movgr2fcsr fcsr0, \tmp0
#ifdef CONFIG_CPU_HAS_LBT
    andi \tmp0, \tmp0, FPU_CSR_TM
    beqz \tmp0, 2f
    ld.w \tmp0, \thread, THREAD_FTOP
    andi \tmp0, \tmp0, 0x7
    la.pcrel \tmp1, 1f
    alsl.d \tmp1, \tmp0, \tmp1, 3
    jr \tmp1
1: x86mttop 0
    b 2f
    x86mttop 1
    b 2f
    x86mttop 2
    b 2f
    x86mttop 3
    b 2f
    x86mttop 4
    b 2f
    x86mttop 5
    b 2f
    x86mttop 6
    b 2f
    x86mttop 7
2:
#endif
.endm

/**
 * @brief 保存FPU条件码
 * @param thread 线程结构体指针
 * @param tmp0 临时寄存器0
 * @param tmp1 临时寄存器1
 */
.macro fpu_save_cc thread, tmp0, tmp1
    movcf2gr \tmp0, $fcc0
    move \tmp1, \tmp0
    movcf2gr \tmp0, $fcc1
    bstrins.d \tmp1, \tmp0, 15, 8
    movcf2gr \tmp0, $fcc2
    bstrins.d \tmp1, \tmp0, 23, 16
    movcf2gr \tmp0, $fcc3
    bstrins.d \tmp1, \tmp0, 31, 24
    movcf2gr \tmp0, $fcc4
    bstrins.d \tmp1, \tmp0, 39, 32
    movcf2gr \tmp0, $fcc5
    bstrins.d \tmp1, \tmp0, 47, 40
    movcf2gr \tmp0, $fcc6
    bstrins.d \tmp1, \tmp0, 55, 48
    movcf2gr \tmp0, $fcc7
    bstrins.d \tmp1, \tmp0, 63, 56
    st.d \tmp1, \thread, THREAD_FCC
.endm

/**
 * @brief 恢复FPU条件码
 * @param thread 线程结构体指针
 * @param tmp0 临时寄存器0
 * @param tmp1 临时寄存器1
 */
.macro fpu_restore_cc thread, tmp0, tmp1
    ldptr.d \tmp0, \thread, THREAD_FCC
    bstrpick.d \tmp1, \tmp0, 7, 0
    movgr2cf $fcc0, \tmp1
    bstrpick.d \tmp1, \tmp0, 15, 8
    movgr2cf $fcc1, \tmp1
    bstrpick.d \tmp1, \tmp0, 23, 16
    movgr2cf $fcc2, \tmp1
    bstrpick.d \tmp1, \tmp0, 31, 24
    movgr2cf $fcc3, \tmp1
    bstrpick.d \tmp1, \tmp0, 39, 32
    movgr2cf $fcc4, \tmp1
    bstrpick.d \tmp1, \tmp0, 47, 40
    movgr2cf $fcc5, \tmp1
    bstrpick.d \tmp1, \tmp0, 55, 48
    movgr2cf $fcc6, \tmp1
    bstrpick.d \tmp1, \tmp0, 63, 56
    movgr2cf $fcc7, \tmp1
.endm

/**
 * @brief 保存双精度浮点寄存器
 * @param thread 线程结构体指针
 * @param tmp 临时寄存器
 */
.macro fpu_save_double thread, tmp
    li.w \tmp, THREAD_FPR0
    PTR_ADD \tmp, \tmp, \thread
    fst.d $f0, \tmp, THREAD_FPR0 - THREAD_FPR0
    fst.d $f1, \tmp, THREAD_FPR1 - THREAD_FPR0
    fst.d $f2, \tmp, THREAD_FPR2 - THREAD_FPR0
    fst.d $f3, \tmp, THREAD_FPR3 - THREAD_FPR0
    fst.d $f4, \tmp, THREAD_FPR4 - THREAD_FPR0
    fst.d $f5, \tmp, THREAD_FPR5 - THREAD_FPR0
    fst.d $f6, \tmp, THREAD_FPR6 - THREAD_FPR0
    fst.d $f7, \tmp, THREAD_FPR7 - THREAD_FPR0
    fst.d $f8, \tmp, THREAD_FPR8 - THREAD_FPR0
    fst.d $f9, \tmp, THREAD_FPR9 - THREAD_FPR0
    fst.d $f10, \tmp, THREAD_FPR10 - THREAD_FPR0
    fst.d $f11, \tmp, THREAD_FPR11 - THREAD_FPR0
    fst.d $f12, \tmp, THREAD_FPR12 - THREAD_FPR0
    fst.d $f13, \tmp, THREAD_FPR13 - THREAD_FPR0
    fst.d $f14, \tmp, THREAD_FPR14 - THREAD_FPR0
    fst.d $f15, \tmp, THREAD_FPR15 - THREAD_FPR0
    fst.d $f16, \tmp, THREAD_FPR16 - THREAD_FPR0
    fst.d $f17, \tmp, THREAD_FPR17 - THREAD_FPR0
    fst.d $f18, \tmp, THREAD_FPR18 - THREAD_FPR0
    fst.d $f19, \tmp, THREAD_FPR19 - THREAD_FPR0
    fst.d $f20, \tmp, THREAD_FPR20 - THREAD_FPR0
    fst.d $f21, \tmp, THREAD_FPR21 - THREAD_FPR0
    fst.d $f22, \tmp, THREAD_FPR22 - THREAD_FPR0
    fst.d $f23, \tmp, THREAD_FPR23 - THREAD_FPR0
    fst.d $f24, \tmp, THREAD_FPR24 - THREAD_FPR0
    fst.d $f25, \tmp, THREAD_FPR25 - THREAD_FPR0
    fst.d $f26, \tmp, THREAD_FPR26 - THREAD_FPR0
    fst.d $f27, \tmp, THREAD_FPR27 - THREAD_FPR0
    fst.d $f28, \tmp, THREAD_FPR28 - THREAD_FPR0
    fst.d $f29, \tmp, THREAD_FPR29 - THREAD_FPR0
    fst.d $f30, \tmp, THREAD_FPR30 - THREAD_FPR0
    fst.d $f31, \tmp, THREAD_FPR31 - THREAD_FPR0
.endm

/**
 * @brief 恢复双精度浮点寄存器
 * @param thread 线程结构体指针
 * @param tmp 临时寄存器
 */
.macro fpu_restore_double thread, tmp
    li.w \tmp, THREAD_FPR0
    PTR_ADD \tmp, \tmp, \thread
    fld.d $f0, \tmp, THREAD_FPR0 - THREAD_FPR0
    fld.d $f1, \tmp, THREAD_FPR1 - THREAD_FPR0
    fld.d $f2, \tmp, THREAD_FPR2 - THREAD_FPR0
    fld.d $f3, \tmp, THREAD_FPR3 - THREAD_FPR0
    fld.d $f4, \tmp, THREAD_FPR4 - THREAD_FPR0
    fld.d $f5, \tmp, THREAD_FPR5 - THREAD_FPR0
    fld.d $f6, \tmp, THREAD_FPR6 - THREAD_FPR0
    fld.d $f7, \tmp, THREAD_FPR7 - THREAD_FPR0
    fld.d $f8, \tmp, THREAD_FPR8 - THREAD_FPR0
    fld.d $f9, \tmp, THREAD_FPR9 - THREAD_FPR0
    fld.d $f10, \tmp, THREAD_FPR10 - THREAD_FPR0
    fld.d $f11, \tmp, THREAD_FPR11 - THREAD_FPR0
    fld.d $f12, \tmp, THREAD_FPR12 - THREAD_FPR0
    fld.d $f13, \tmp, THREAD_FPR13 - THREAD_FPR0
    fld.d $f14, \tmp, THREAD_FPR14 - THREAD_FPR0
    fld.d $f15, \tmp, THREAD_FPR15 - THREAD_FPR0
    fld.d $f16, \tmp, THREAD_FPR16 - THREAD_FPR0
    fld.d $f17, \tmp, THREAD_FPR17 - THREAD_FPR0
    fld.d $f18, \tmp, THREAD_FPR18 - THREAD_FPR0
    fld.d $f19, \tmp, THREAD_FPR19 - THREAD_FPR0
    fld.d $f20, \tmp, THREAD_FPR20 - THREAD_FPR0
    fld.d $f21, \tmp, THREAD_FPR21 - THREAD_FPR0
    fld.d $f22, \tmp, THREAD_FPR22 - THREAD_FPR0
    fld.d $f23, \tmp, THREAD_FPR23 - THREAD_FPR0
    fld.d $f24, \tmp, THREAD_FPR24 - THREAD_FPR0
    fld.d $f25, \tmp, THREAD_FPR25 - THREAD_FPR0
    fld.d $f26, \tmp, THREAD_FPR26 - THREAD_FPR0
    fld.d $f27, \tmp, THREAD_FPR27 - THREAD_FPR0
    fld.d $f28, \tmp, THREAD_FPR28 - THREAD_FPR0
    fld.d $f29, \tmp, THREAD_FPR29 - THREAD_FPR0
    fld.d $f30, \tmp, THREAD_FPR30 - THREAD_FPR0
    fld.d $f31, \tmp, THREAD_FPR31 - THREAD_FPR0
.endm

/** @} */

/*************************** 任务上下文 ***************************/

/**
 * @defgroup TaskContextMacros 任务上下文保存/恢复宏
 * @{
 */

/**
 * @brief 保存任务上下文
 * @param thread 线程结构体指针
 */
.macro task_context_save thread
    st.d ra, \thread, THREAD_REG1
    st.d tp, \thread, THREAD_REG2
    st.d sp, \thread, THREAD_REG3
    st.d a2, \thread, THREAD_REG6
    st.d a3, \thread, THREAD_REG7
    st.d a4, \thread, THREAD_REG8
    st.d a5, \thread, THREAD_REG9
    st.d a6, \thread, THREAD_REG10
    st.d a7, \thread, THREAD_REG11
    st.d t0, \thread, THREAD_REG12
    st.d t1, \thread, THREAD_REG13
    st.d t2, \thread, THREAD_REG14
    st.d t3, \thread, THREAD_REG15
    st.d t4, \thread, THREAD_REG16
    st.d t5, \thread, THREAD_REG17
    st.d t6, \thread, THREAD_REG18
    st.d t7, \thread, THREAD_REG19
    st.d t8, \thread, THREAD_REG20
    st.d u0, \thread, THREAD_REG21
    st.d fp, \thread, THREAD_REG22
    st.d s0, \thread, THREAD_REG23
    st.d s1, \thread, THREAD_REG24
    st.d s2, \thread, THREAD_REG25
    st.d s3, \thread, THREAD_REG26
    st.d s4, \thread, THREAD_REG27
    st.d s5, \thread, THREAD_REG28
    st.d s6, \thread, THREAD_REG29
    st.d s7, \thread, THREAD_REG30
    st.d s8, \thread, THREAD_REG31
.endm

/**
 * @brief 恢复任务上下文
 * @param thread 线程结构体指针
 */
.macro task_context_restore thread
    ld.d ra, \thread, THREAD_REG1
    ld.d tp, \thread, THREAD_REG2
    ld.d sp, \thread, THREAD_REG3
    ld.d a2, \thread, THREAD_REG6
    ld.d a3, \thread, THREAD_REG7
    ld.d a4, \thread, THREAD_REG8
    ld.d a5, \thread, THREAD_REG9
    ld.d a6, \thread, THREAD_REG10
    ld.d a7, \thread, THREAD_REG11
    ld.d t0, \thread, THREAD_REG12
    ld.d t1, \thread, THREAD_REG13
    ld.d t2, \thread, THREAD_REG14
    ld.d t3, \thread, THREAD_REG15
    ld.d t4, \thread, THREAD_REG16
    ld.d t5, \thread, THREAD_REG17
    ld.d t6, \thread, THREAD_REG18
    ld.d t7, \thread, THREAD_REG19
    ld.d t8, \thread, THREAD_REG20
    ld.d u0, \thread, THREAD_REG21
    ld.d fp, \thread, THREAD_REG22
    ld.d s0, \thread, THREAD_REG23
    ld.d s1, \thread, THREAD_REG24
    ld.d s2, \thread, THREAD_REG25
    ld.d s3, \thread, THREAD_REG26
    ld.d s4, \thread, THREAD_REG27
    ld.d s5, \thread, THREAD_REG28
    ld.d s6, \thread, THREAD_REG29
    ld.d s7, \thread, THREAD_REG30
    ld.d s8, \thread, THREAD_REG31
.endm

/** @} */

#endif /* ASM_USE */

/************************C++兼容性******************************/
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASM_H */
