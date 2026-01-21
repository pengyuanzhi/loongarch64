#define ASM_USE
#include <cpu.h>
#include <asm.h>
#incldue <asrspace.h>
	.extern handle_syscall
	.align	5
ENTRY(__arch_cpu_idle)
	/* start of idle interrupt region */
	ori	t0, zero, CSR_CRMD_IE
	/* idle instruction needs irq enabled */
	csrxchg	t0, t0, LOONGARCH_CSR_CRMD
	/*
	 * If an interrupt lands here; between enabling interrupts above and
	 * going idle on the next instruction, we must *NOT* go idle since the
	 * interrupt could have set TIF_NEED_RESCHED or caused an timer to need
	 * reprogramming. Fall through -- see handle_vint() below -- and have
	 * the idle loop take care of things.
	 */
	idle	0
	/* end of idle interrupt region */
1:	jr	ra
ENDPROC(__arch_cpu_idle)
ENTRY(handle_vint)
	SAVE_ALL
	la_abs	t1, 1b
	LONG_L	t0, sp, PT_ERA
	/* 3 instructions idle interrupt region */
	ori	t0, t0, 0b1100
	bne	t0, t1, 1f
	LONG_S	t0, sp, PT_ERA
1:	move	a0, sp
	move	a1, sp
	la_abs	t0, do_irq
	jirl	ra, t0, 0
	RESTORE_ALL_AND_RET
ENDPROC(handle_vint)
	.macro	build_prep_badv
	csrrd	t0, LOONGARCH_CSR_BADV
	PTR_S	t0, sp, PT_BVADDR
	.endm
	.macro	build_prep_fcsr
	movfcsr2gr	a1, fcsr0
	.endm
	.macro	build_prep_none
	.endm
	.macro	BUILD_HANDLER exception handler prep
	.align	5
	ENTRY(handle_\exception)
	666:
	SAVE_ALL
	build_prep_\prep
	move	a0, sp
	la_abs	t0, do_\handler
	jirl	ra, t0, 0
	668:
	RESTORE_ALL_AND_RET
	ENDPROC(handle_\exception)
	.pushsection	".data", "aw", %progbits
	.popsection
	.endm
	BUILD_HANDLER ade ade badv
	BUILD_HANDLER ale ale badv
	BUILD_HANDLER bce bce none
	BUILD_HANDLER bp bp none
	BUILD_HANDLER fpe fpe fcsr
	BUILD_HANDLER fpu fpu none
	BUILD_HANDLER lsx lsx none
	BUILD_HANDLER lasx lasx none
	BUILD_HANDLER lbt lbt none
	BUILD_HANDLER ri ri none
	BUILD_HANDLER watch watch none
//	BUILD_HANDLER reserved reserved none	/* others */
ENTRY(handle_sys)
	la_abs	t0, handle_syscall
	jr	t0
ENDPROC(handle_sys)
/* handle_reserved */
ENTRY(handle_reserved)
/* TODO LOONGARCH */
	li.d     t1,0x800000001fe001e0
	li.w     t2,0x49
	st.b     t2,t1,0
	ertn
ENDPROC(handle_reserved)
