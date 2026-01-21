/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Some useful macros for LoongArch assembler code
 *
 * Copyright (C) 2020-2022 Loongson Technology Corporation Limited
 *
 * Derived from MIPS:
 * Copyright (C) 1995, 1996, 1997, 1999, 2001 by Ralf Baechle
 * Copyright (C) 1999 by Silicon Graphics, Inc.
 * Copyright (C) 2001 MIPS Technologies, Inc.
 * Copyright (C) 2002  Maciej W. Rozycki
 */
#ifndef _ASM_H
#define _ASM_H

#include <asm-offsets.h>
/************************头文件********************************/
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/*
*  LoongArch register definitions
*/
#define zero	$r0	/* wired zero */
#define ra	$r1	/* return address */
#define tp	$r2 /* thread pointer */
#define gp	$r2 /* thread pointer ? */
#define sp	$r3	/* stack pointer */
#define v0	$r4	/* return value - caller saved */
#define v1	$r5
#define a0	$r4	/* argument registers */
#define a1	$r5
#define a2	$r6
#define a3	$r7
#define a4	$r8
#define a5	$r9
#define a6	$r10
#define a7	$r11
#define t0	$r12	/* caller saved func myself use*/
#define t1	$r13
#define t2	$r14
#define t3	$r15
#define t4	$r16
#define t5	$r17
#define t6	$r18
#define t7	$r19
#define t8	$r20
/* $r21: Temporarily reserved */
#define u0  $r21     /* assembler temp */
#define fp	$r22	/* frame pointer */

#define s0	$r23	/* callee saved  if func use this ,need save original data . at the end of func must recover original data */
#define s1	$r24
#define s2	$r25
#define s3	$r26
#define s4	$r27
#define s5	$r28
#define s6	$r29
#define s7	$r30
#define s8	$r31



/*
*  LOONGARCH floating point coprocessor register definitions
*/

#define fp0     $f0     /* return reg 0 */
#define fp1     $f1     /* return reg 1 */
#define fp2     $f2     /* return reg 2 */
#define fp3     $f3     /* return reg 3 */
#define fp4     $f4     /* caller saved 0 */
#define fp5     $f5     /* caller saved 1 */
#define fp6     $f6     /* caller saved 2 */
#define fp7     $f7     /* caller saved 3 */
#define fp8     $f8     /* caller saved 4 */
#define fp9     $f9     /* caller saved 5 */
#define fp10    $f10    /* caller saved 6 */
#define fp11    $f11    /* caller saved 7 */
#define fp12    $f12    /* arg reg 0 */
#define fp13    $f13    /* arg reg 1 */
#define fp14    $f14    /* arg reg 2 */
#define fp15    $f15    /* arg reg 3 */
#define fp16    $f16    /* caller saved 8 */
#define fp17    $f17    /* caller saved 9 */
#define fp18    $f18    /* caller saved 10 */
#define fp19    $f19    /* caller saved 11 */
#define fp20    $f20    /* callee saved 0 */
#define fp21    $f21    /* callee saved 1 */
#define fp22    $f22    /* callee saved 2 */
#define fp23    $f23    /* callee saved 3 */
#define fp24    $f24    /* callee saved 4 */
#define fp25    $f25    /* callee saved 5 */
#define fp26    $f26    /* callee saved 6 */
#define fp27    $f27    /* callee saved 7 */
#define fp28    $f28    /* callee saved 8 */
#define fp29    $f29    /* callee saved 9 */
#define fp30    $f30    /* callee saved 10 */
#define fp31    $f31    /* callee saved 11 */


#define fcsr0   $r0
#define fcsr1   $r1
#define fcsr2   $r2
#define fcsr3   $r3
#if 0
#define fcc0    $fcc0
#define fcc1    $fcc1
#define fcc2    $fcc2
#define fcc3    $fcc3
#define fcc4    $fcc4
#define fcc5    $fcc5
#define fcc6    $fcc6
#define fcc7    $fcc7
#endif

/*
 * Size of a register
 */
#ifndef __loongarch64
#define SZREG	4
#else
#define SZREG	8
#endif

#define LONGSIZE	8

/* LoongArch pref instruction. */
#ifdef CONFIG_CPU_HAS_PREFETCH

#define PREF(hint, addr, offs)				\
		preld	hint, addr, offs;		\

#define PREFX(hint, addr, index)			\
		preldx	hint, addr, index;		\

#else /* !CONFIG_CPU_HAS_PREFETCH */

#define PREF(hint, addr, offs)
#define PREFX(hint, addr, index)

#endif /* !CONFIG_CPU_HAS_PREFETCH */

/*
 * Stack alignment
 */
#define STACK_ALIGN	~(0xf)

/*
 * Macros to handle different pointer/register sizes for 32/64-bit code
 */

/*
 * Size of a register
 */
#ifndef __loongarch64
#define SZREG	4
#else
#define SZREG	8
#endif

/*
 * Use the following macros in assemblercode to load/store registers,
 * pointers etc.
 */
#if (SZREG == 4)
#define REG_L		ld.w
#define REG_S		st.w
#define REG_ADD		add.w
#define REG_SUB		sub.w
#else /* SZREG == 8 */
#define REG_L		ld.d
#define REG_S		st.d
#define REG_ADD		add.d
#define REG_SUB		sub.d
#endif

/*
 * How to add/sub/load/store/shift C int variables.
 */
#if (__SIZEOF_INT__ == 4)
#define INT_ADD		add.w
#define INT_ADDI	addi.w
#define INT_SUB		sub.w
#define INT_L		ld.w
#define INT_S		st.w
#define INT_SLL		slli.w
#define INT_SLLV	sll.w
#define INT_SRL		srli.w
#define INT_SRLV	srl.w
#define INT_SRA		srai.w
#define INT_SRAV	sra.w
#endif

#if (__SIZEOF_INT__ == 8)
#define INT_ADD		add.d
#define INT_ADDI	addi.d
#define INT_SUB		sub.d
#define INT_L		ld.d
#define INT_S		st.d
#define INT_SLL		slli.d
#define INT_SLLV	sll.d
#define INT_SRL		srli.d
#define INT_SRLV	srl.d
#define INT_SRA		srai.d
#define INT_SRAV	sra.d
#endif

/*
 * How to add/sub/load/store/shift C long variables.
 */
#if (__SIZEOF_LONG__ == 4)
#define LONG_ADD	add.w
#define LONG_ADDI	addi.w
#define LONG_SUB	sub.w
#define LONG_L		ld.w
#define LONG_S		st.w
#define LONG_SLL	slli.w
#define LONG_SLLV	sll.w
#define LONG_SRL	srli.w
#define LONG_SRLV	srl.w
#define LONG_SRA	srai.w
#define LONG_SRAV	sra.w

#ifdef ASM_USE
#define LONG		.word
#endif
#define LONGSIZE	4
#define LONGMASK	3
#define LONGLOG		2
#endif

#if (__SIZEOF_LONG__ == 8)
#define LONG_ADD	add.d
#define LONG_ADDI	addi.d
#define LONG_SUB	sub.d
#define LONG_L		ld.d
#define LONG_S		st.d
#define LONG_SLL	slli.d
#define LONG_SLLV	sll.d
#define LONG_SRL	srli.d
#define LONG_SRLV	srl.d
#define LONG_SRA	srai.d
#define LONG_SRAV	sra.d

#ifdef ASM_USE
#define LONG		.dword
#endif
#define LONGSIZE	8
#define LONGMASK	7
#define LONGLOG		3
#endif

/*
 * How to add/sub/load/store/shift pointers.
 */
#if (__SIZEOF_POINTER__ == 4)
#define PTR_ADD		add.w
#define PTR_ADDI	addi.w
#define PTR_SUB		sub.w
#define PTR_L		ld.w
#define PTR_S		st.w
#define PTR_LI		li.w
#define PTR_SLL		slli.w
#define PTR_SLLV	sll.w
#define PTR_SRL		srli.w
#define PTR_SRLV	srl.w
#define PTR_SRA		srai.w
#define PTR_SRAV	sra.w

#define PTR_SCALESHIFT	2

#ifdef ASM_USE
#define PTR		.word
#endif
#define PTRSIZE		4
#define PTRLOG		2
#endif

#if (__SIZEOF_POINTER__ == 8)
#define PTR_ADD		add.d
#define PTR_ADDI	addi.d
#define PTR_SUB		sub.d
#define PTR_L		ld.d
#define PTR_S		st.d
#define PTR_LI		li.d
#define PTR_SLL		slli.d
#define PTR_SLLV	sll.d
#define PTR_SRL		srli.d
#define PTR_SRLV	srl.d
#define PTR_SRA		srai.d
#define PTR_SRAV	sra.d

#define PTR_SCALESHIFT	3

#ifdef ASM_USE
#define PTR		.dword
#endif
#define PTRSIZE		8
#define PTRLOG		3
#endif

/* Annotate a function as being unsuitable for kprobes. */
#ifdef CONFIG_KPROBES
#define _ASM_NOKPROBE(name)				\
	.pushsection "_kprobe_blacklist", "aw";		\
	.quad	name;					\
	.popsection
#else
#define _ASM_NOKPROBE(name)
#endif

#ifdef ASM_USE

#define ALIGN .align 4

#define ENTRY(name)                             \
  .globl name;                                  \
  ALIGN;                                        \
  name:

#define GLOBAL(name)                            \
  .globl name;                                  \
  name:
  
#define END(name) \
  .size name, .-name
  
#define ENDPROC(name) \
  .type name, %function; \
  END(name)



	.macro BACKUP_T0T1
	csrwr	t0, EXCEPTION_KS0
	csrwr	t1, EXCEPTION_KS1
	.endm

	.macro RELOAD_T0T1
	csrrd   t0, EXCEPTION_KS0
	csrrd   t1, EXCEPTION_KS1
	.endm


/*
 * get_saved_sp returns the SP for the current CPU by looking in the
 * kernelsp array for it. It stores the current sp in t0 and loads the
 * new value in sp.
 */
	.macro	get_saved_sp
#ifdef CONFIG_SMP
    csrrd     t0, LOONGARCH_CSR_CPUID
    li.w      t1, __SIZEOF_POINTER__
    mul.w     t0, t0, t1
#endif
	la_abs	  t1, running_task_stack
#ifdef CONFIG_SMP
    LONG_ADD  t1, t1, t0
#endif
	move	  t0, sp
	LONG_L	  sp, t1, 0
	.endm

	.macro	set_saved_sp stackp temp temp2
	la.pcrel  \temp, kernelsp
#ifdef CONFIG_SMP
	LONG_ADD  \temp, \temp, u0
#endif
	LONG_S	  \stackp, \temp, 0
	.endm

	.macro	SAVE_SOME
	csrrd	t1, LOONGARCH_CSR_PRMD
	andi	t1, t1, 0x3	/* extract pplv bit */
	move	t0, sp
	beqz	t1, 0f
	/* Called from user mode, new stack. */
	get_saved_sp
0:	
	PTR_ADDI  sp,sp,-PT_SIZE
	LONG_S	zero,sp,PT_R0
	LONG_S	ra,sp,PT_R1
	LONG_S	tp,sp,PT_R2
	LONG_S  t0,sp,PT_R3
	LONG_S	a0,sp,PT_R4
	LONG_S	a1,sp,PT_R5
	LONG_S	a2,sp,PT_R6
	LONG_S	a3,sp,PT_R7
	LONG_S	a4,sp,PT_R8
	LONG_S	a5,sp,PT_R9
	LONG_S	a6,sp,PT_R10
	LONG_S	a7,sp,PT_R11
	LONG_S	fp,sp,PT_R22
	csrrd	ra,LOONGARCH_CSR_ERA
	LONG_S	ra,sp, PT_ERA
	csrrd	t0,LOONGARCH_CSR_PRMD
	LONG_S	t0,sp,PT_PRMD
	csrrd	t0,LOONGARCH_CSR_CRMD
	LONG_S	t0,sp,PT_CRMD
	csrrd	t0,LOONGARCH_CSR_EUEN
	LONG_S  t0,sp,PT_EUEN
	csrrd	t0,LOONGARCH_CSR_ECFG
	LONG_S	t0,sp,PT_ECFG
	csrrd	t0,LOONGARCH_CSR_ESTAT
	PTR_S	t0,sp,PT_ESTAT
	/* Set thread_info if we're coming from user mode */
	csrrd	t0, LOONGARCH_CSR_PRMD
	andi	t0, t0, 0x3	/* extract pplv bit */
	beqz	t0, 1f
/*
	li.d	tp, ~_THREAD_MASK
	and		tp, tp, sp
*/
	LONG_S	u0,sp,PT_R21
/*	csrrd	u0, PERCPU_BASE_KS */
1:
	.endm

	.macro	SAVE_TEMP
	LONG_S	t0,sp,PT_R12
	LONG_S	t1,sp,PT_R13
	LONG_S	t2,sp,PT_R14
	LONG_S	t3,sp,PT_R15
	LONG_S	t4,sp,PT_R16
	LONG_S	t5,sp,PT_R17
	LONG_S	t6,sp,PT_R18
	LONG_S	t7,sp,PT_R19
	LONG_S	t8,sp,PT_R20
	.endm

	.macro	SAVE_STATIC
	LONG_S	s0,sp,PT_R23
	LONG_S	s1,sp,PT_R24
	LONG_S	s2,sp,PT_R25
	LONG_S	s3,sp,PT_R26
	LONG_S	s4,sp,PT_R27
	LONG_S	s5,sp,PT_R28
	LONG_S	s6,sp,PT_R29
	LONG_S	s7,sp,PT_R30
	LONG_S	s8,sp,PT_R31
	.endm

	.macro	SAVE_ALL
	BACKUP_T0T1
	SAVE_SOME
	RELOAD_T0T1
	SAVE_TEMP
	SAVE_STATIC
	.endm


	.macro	RESTORE_TEMP
	LONG_L	t0,sp,PT_R12
	LONG_L	t1,sp,PT_R13
	LONG_L	t2,sp,PT_R14
	LONG_L	t3,sp,PT_R15
	LONG_L	t4,sp,PT_R16
	LONG_L	t5,sp,PT_R17
	LONG_L	t6,sp,PT_R18
	LONG_L	t7,sp,PT_R19
	LONG_L	t8,sp,PT_R20
	.endm

	.macro	RESTORE_STATIC
	LONG_L	s0,sp,PT_R23
	LONG_L	s1,sp,PT_R24
	LONG_L	s2,sp,PT_R25
	LONG_L	s3,sp,PT_R26
	LONG_L	s4,sp,PT_R27
	LONG_L	s5,sp,PT_R28
	LONG_L	s6,sp,PT_R29
	LONG_L	s7,sp,PT_R30
	LONG_L	s8,sp,PT_R31
	.endm

	.macro	RESTORE_SOME
    /* TODO:jcai csr_estat/csr_ecfg/csr_euen/csr_crmd 寄存器的值为何不恢复？ */
	LONG_L	a0, sp, PT_ERA
	csrwr	a0, LOONGARCH_CSR_ERA
	LONG_L	a0, sp, PT_PRMD
	csrwr	a0, LOONGARCH_CSR_PRMD
	andi    a0, a0, 0x3	/* extract pplv bit */
	beqz    a0, 0f
	LONG_L  u0, sp,PT_R21   /* 在 csr_prmd 的 PPLV 不为0时，表示当前需要返回用户空间了，所以恢复 u0/r21 寄存器的值 */
0:
	LONG_L	ra,sp,PT_R1
	LONG_L	tp,sp,PT_R2
	LONG_L	a0,sp,PT_R4
	LONG_L	a1,sp,PT_R5
	LONG_L	a2,sp,PT_R6
	LONG_L	a3,sp,PT_R7
	LONG_L	a4,sp,PT_R8
	LONG_L	a5,sp,PT_R9
	LONG_L	a6,sp,PT_R10
	LONG_L	a7,sp,PT_R11
	LONG_L	fp,sp,PT_R22
	.endm

	.macro	RESTORE_SP_AND_RET
	LONG_L	sp,sp,PT_R3
	ertn
	.endm

	.macro	RESTORE_ALL_AND_RET
	RESTORE_STATIC
	RESTORE_TEMP
	RESTORE_SOME
	RESTORE_SP_AND_RET
	.endm

	.macro la_abs reg, sym
#ifndef CONFIG_RELOCATABLE
	la.abs	\reg, \sym
#else
	766:
	lu12i.w	\reg, 0
	ori	\reg, \reg, 0
	lu32i.d	\reg, 0
	lu52i.d	\reg, \reg, 0
	.pushsection ".la_abs", "aw", %progbits
	.p2align 3
	.dword	766b
	.dword	\sym
	.popsection
#endif
.endm

#define __ASM_EXTABLE_RAW(insn, fixup, type, data)	\
	.pushsection	__ex_table, "a";		\
	.balign		4;				\
	.long		((insn) - .);			\
	.long		((fixup) - .);			\
	.short		(type);				\
	.short		(data);				\
	.popsection;

	.macro		_asm_extable, insn, fixup
	__ASM_EXTABLE_RAW(\insn, \fixup, EX_TYPE_FIXUP, 0)
	.endm

	.macro fpu_save_csr thread tmp
	movfcsr2gr	\tmp, fcsr0
	st.d		\tmp, \thread, THREAD_FCSR
#ifdef CONFIG_CPU_HAS_LBT
	/* TM bit is always 0 if LBT not supported */
	andi		\tmp, \tmp, FPU_CSR_TM
	beqz		\tmp, 1f
	/* Save FTOP */
	x86mftop	\tmp
	st.w		\tmp, \thread, THREAD_FTOP
	/* Turn off TM to ensure the order of FPR in memory independent of TM */
	x86clrtm
1:
#endif
	.endm

	.macro fpu_restore_csr thread tmp0 tmp1
	ld.w		\tmp0, \thread, THREAD_FCSR
	movgr2fcsr	fcsr0, \tmp0
#ifdef CONFIG_CPU_HAS_LBT
	/* TM bit is always 0 if LBT not supported */
	andi		\tmp0, \tmp0, FPU_CSR_TM
	beqz		\tmp0, 2f
	/* Restore FTOP */
	ld.w		\tmp0, \thread, THREAD_FTOP
	andi		\tmp0, \tmp0, 0x7
	la.pcrel	\tmp1, 1f
	alsl.d		\tmp1, \tmp0, \tmp1, 3
	jr		\tmp1
1:
	x86mttop	0
	b	2f
	x86mttop	1
	b	2f
	x86mttop	2
	b	2f
	x86mttop	3
	b	2f
	x86mttop	4
	b	2f
	x86mttop	5
	b	2f
	x86mttop	6
	b	2f
	x86mttop	7
2:
#endif
	.endm

.macro fpu_save_cc thread tmp0 tmp1
	movcf2gr	\tmp0, $fcc0
	move	\tmp1, \tmp0
	movcf2gr	\tmp0, $fcc1
	bstrins.d	\tmp1, \tmp0, 15, 8
	movcf2gr	\tmp0, $fcc2
	bstrins.d	\tmp1, \tmp0, 23, 16
	movcf2gr	\tmp0, $fcc3
	bstrins.d	\tmp1, \tmp0, 31, 24
	movcf2gr	\tmp0, $fcc4
	bstrins.d	\tmp1, \tmp0, 39, 32
	movcf2gr	\tmp0, $fcc5
	bstrins.d	\tmp1, \tmp0, 47, 40
	movcf2gr	\tmp0, $fcc6
	bstrins.d	\tmp1, \tmp0, 55, 48
	movcf2gr	\tmp0, $fcc7
	bstrins.d	\tmp1, \tmp0, 63, 56
	st.d		\tmp1, \thread, THREAD_FCC
	.endm

	.macro fpu_restore_cc thread tmp0 tmp1
	ldptr.d	\tmp0, \thread, THREAD_FCC
	bstrpick.d	\tmp1, \tmp0, 7, 0
	movgr2cf	$fcc0, \tmp1
	bstrpick.d	\tmp1, \tmp0, 15, 8
	movgr2cf	$fcc1, \tmp1
	bstrpick.d	\tmp1, \tmp0, 23, 16
	movgr2cf	$fcc2, \tmp1
	bstrpick.d	\tmp1, \tmp0, 31, 24
	movgr2cf	$fcc3, \tmp1
	bstrpick.d	\tmp1, \tmp0, 39, 32
	movgr2cf	$fcc4, \tmp1
	bstrpick.d	\tmp1, \tmp0, 47, 40
	movgr2cf	$fcc5, \tmp1
	bstrpick.d	\tmp1, \tmp0, 55, 48
	movgr2cf	$fcc6, \tmp1
	bstrpick.d	\tmp1, \tmp0, 63, 56
	movgr2cf	$fcc7, \tmp1
	.endm

.macro	fpu_save_double thread tmp
	li.w	\tmp, THREAD_FPR0
	PTR_ADD \tmp, \tmp, \thread
	fst.d	$f0, \tmp, THREAD_FPR0  - THREAD_FPR0
	fst.d	$f1, \tmp, THREAD_FPR1  - THREAD_FPR0
	fst.d	$f2, \tmp, THREAD_FPR2  - THREAD_FPR0
	fst.d	$f3, \tmp, THREAD_FPR3  - THREAD_FPR0
	fst.d	$f4, \tmp, THREAD_FPR4  - THREAD_FPR0
	fst.d	$f5, \tmp, THREAD_FPR5  - THREAD_FPR0
	fst.d	$f6, \tmp, THREAD_FPR6  - THREAD_FPR0
	fst.d	$f7, \tmp, THREAD_FPR7  - THREAD_FPR0
	fst.d	$f8, \tmp, THREAD_FPR8  - THREAD_FPR0
	fst.d	$f9, \tmp, THREAD_FPR9  - THREAD_FPR0
	fst.d	$f10, \tmp, THREAD_FPR10 - THREAD_FPR0
	fst.d	$f11, \tmp, THREAD_FPR11 - THREAD_FPR0
	fst.d	$f12, \tmp, THREAD_FPR12 - THREAD_FPR0
	fst.d	$f13, \tmp, THREAD_FPR13 - THREAD_FPR0
	fst.d	$f14, \tmp, THREAD_FPR14 - THREAD_FPR0
	fst.d	$f15, \tmp, THREAD_FPR15 - THREAD_FPR0
	fst.d	$f16, \tmp, THREAD_FPR16 - THREAD_FPR0
	fst.d	$f17, \tmp, THREAD_FPR17 - THREAD_FPR0
	fst.d	$f18, \tmp, THREAD_FPR18 - THREAD_FPR0
	fst.d	$f19, \tmp, THREAD_FPR19 - THREAD_FPR0
	fst.d	$f20, \tmp, THREAD_FPR20 - THREAD_FPR0
	fst.d	$f21, \tmp, THREAD_FPR21 - THREAD_FPR0
	fst.d	$f22, \tmp, THREAD_FPR22 - THREAD_FPR0
	fst.d	$f23, \tmp, THREAD_FPR23 - THREAD_FPR0
	fst.d	$f24, \tmp, THREAD_FPR24 - THREAD_FPR0
	fst.d	$f25, \tmp, THREAD_FPR25 - THREAD_FPR0
	fst.d	$f26, \tmp, THREAD_FPR26 - THREAD_FPR0
	fst.d	$f27, \tmp, THREAD_FPR27 - THREAD_FPR0
	fst.d	$f28, \tmp, THREAD_FPR28 - THREAD_FPR0
	fst.d	$f29, \tmp, THREAD_FPR29 - THREAD_FPR0
	fst.d	$f30, \tmp, THREAD_FPR30 - THREAD_FPR0
	fst.d	$f31, \tmp, THREAD_FPR31 - THREAD_FPR0
	.endm

	.macro	fpu_restore_double thread tmp
	li.w	\tmp, THREAD_FPR0
	PTR_ADD \tmp, \tmp, \thread
	fld.d	$f0, \tmp, THREAD_FPR0  - THREAD_FPR0
	fld.d	$f1, \tmp, THREAD_FPR1  - THREAD_FPR0
	fld.d	$f2, \tmp, THREAD_FPR2  - THREAD_FPR0
	fld.d	$f3, \tmp, THREAD_FPR3  - THREAD_FPR0
	fld.d	$f4, \tmp, THREAD_FPR4  - THREAD_FPR0
	fld.d	$f5, \tmp, THREAD_FPR5  - THREAD_FPR0
	fld.d	$f6, \tmp, THREAD_FPR6  - THREAD_FPR0
	fld.d	$f7, \tmp, THREAD_FPR7  - THREAD_FPR0
	fld.d	$f8, \tmp, THREAD_FPR8  - THREAD_FPR0
	fld.d	$f9, \tmp, THREAD_FPR9  - THREAD_FPR0
	fld.d	$f10, \tmp, THREAD_FPR10 - THREAD_FPR0
	fld.d	$f11, \tmp, THREAD_FPR11 - THREAD_FPR0
	fld.d	$f12, \tmp, THREAD_FPR12 - THREAD_FPR0
	fld.d	$f13, \tmp, THREAD_FPR13 - THREAD_FPR0
	fld.d	$f14, \tmp, THREAD_FPR14 - THREAD_FPR0
	fld.d	$f15, \tmp, THREAD_FPR15 - THREAD_FPR0
	fld.d	$f16, \tmp, THREAD_FPR16 - THREAD_FPR0
	fld.d	$f17, \tmp, THREAD_FPR17 - THREAD_FPR0
	fld.d	$f18, \tmp, THREAD_FPR18 - THREAD_FPR0
	fld.d	$f19, \tmp, THREAD_FPR19 - THREAD_FPR0
	fld.d	$f20, \tmp, THREAD_FPR20 - THREAD_FPR0
	fld.d	$f21, \tmp, THREAD_FPR21 - THREAD_FPR0
	fld.d	$f22, \tmp, THREAD_FPR22 - THREAD_FPR0
	fld.d	$f23, \tmp, THREAD_FPR23 - THREAD_FPR0
	fld.d	$f24, \tmp, THREAD_FPR24 - THREAD_FPR0
	fld.d	$f25, \tmp, THREAD_FPR25 - THREAD_FPR0
	fld.d	$f26, \tmp, THREAD_FPR26 - THREAD_FPR0
	fld.d	$f27, \tmp, THREAD_FPR27 - THREAD_FPR0
	fld.d	$f28, \tmp, THREAD_FPR28 - THREAD_FPR0
	fld.d	$f29, \tmp, THREAD_FPR29 - THREAD_FPR0
	fld.d	$f30, \tmp, THREAD_FPR30 - THREAD_FPR0
	fld.d	$f31, \tmp, THREAD_FPR31 - THREAD_FPR0
	.endm

	.macro	task_context_save thread
	st.d	ra, \thread, THREAD_REG1
	st.d	tp, \thread, THREAD_REG2
	st.d	sp, \thread, THREAD_REG3
	//st.d	a0, \thread, THREAD_REG4
	//st.d	a1, \thread, THREAD_REG5
	st.d	a2, \thread, THREAD_REG6
	st.d	a3, \thread, THREAD_REG7
	st.d	a4, \thread, THREAD_REG8
	st.d	a5, \thread, THREAD_REG9
	st.d	a6, \thread, THREAD_REG10
	st.d	a7, \thread, THREAD_REG11
	st.d	t0, \thread, THREAD_REG12
	st.d	t1, \thread, THREAD_REG13
	st.d	t2, \thread, THREAD_REG14
	st.d	t3, \thread, THREAD_REG15
	st.d	t4, \thread, THREAD_REG16
	st.d	t5, \thread, THREAD_REG17
	st.d	t6, \thread, THREAD_REG18
	st.d	t7, \thread, THREAD_REG19
	st.d	t8, \thread, THREAD_REG20
	st.d	u0, \thread, THREAD_REG21
	st.d	fp, \thread, THREAD_REG22
	st.d	s0, \thread, THREAD_REG23
	st.d	s1, \thread, THREAD_REG24
	st.d	s2, \thread, THREAD_REG25
	st.d	s3, \thread, THREAD_REG26
	st.d	s4, \thread, THREAD_REG27
	st.d	s5, \thread, THREAD_REG28
	st.d	s6, \thread, THREAD_REG29
	st.d	s7, \thread, THREAD_REG30
	st.d	s8, \thread, THREAD_REG31
	.endm

	.macro	task_context_restore thread
	ld.d	ra, \thread, THREAD_REG1
	ld.d	tp, \thread, THREAD_REG2
	ld.d	sp, \thread, THREAD_REG3
	//ld.d	a0, \thread, THREAD_REG4
	//ld.d	a1, \thread, THREAD_REG5
	ld.d	a2, \thread, THREAD_REG6
	ld.d	a3, \thread, THREAD_REG7
	ld.d	a4, \thread, THREAD_REG8
	ld.d	a5, \thread, THREAD_REG9
	ld.d	a6, \thread, THREAD_REG10
	ld.d	a7, \thread, THREAD_REG11
	ld.d	t0, \thread, THREAD_REG12
	ld.d	t1, \thread, THREAD_REG13
	ld.d	t2, \thread, THREAD_REG14
	ld.d	t3, \thread, THREAD_REG15
	ld.d	t4, \thread, THREAD_REG16
	ld.d	t5, \thread, THREAD_REG17
	ld.d	t6, \thread, THREAD_REG18
	ld.d	t7, \thread, THREAD_REG19
	ld.d	t8, \thread, THREAD_REG20
	ld.d	u0, \thread, THREAD_REG21
	ld.d	fp, \thread, THREAD_REG22
	ld.d	s0, \thread, THREAD_REG23
	ld.d	s1, \thread, THREAD_REG24
	ld.d	s2, \thread, THREAD_REG25
	ld.d	s3, \thread, THREAD_REG26
	ld.d	s4, \thread, THREAD_REG27
	ld.d	s5, \thread, THREAD_REG28
	ld.d	s6, \thread, THREAD_REG29
	ld.d	s7, \thread, THREAD_REG30
	ld.d	s8, \thread, THREAD_REG31
	.endm

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ASM_ASM_H */
