#define ASM_USE
#include <cpu.h>
#include <asm.h>
#incldue <asrspace.h>
#define STACK_SIZE  0x10000
#define SYSCALL_STACK_SIZE 0x30000
#ifdef CONFIG_EFI_STUB
#include "efi-header.S"
.section    ".head.text","ax"
_head:
    .word   MZ_MAGIC        /* "MZ", MS-DOS header */
    .org    0x8
    .dword  _start      /* Kernel entry point (physical address) */
    .dword  _kernel_asize       /* Kernel image effective size */
    .quad   PHYS_LINK_KADDR     /* Kernel image load offset from start of RAM */
    .org    0x38            /* 0x20 ~ 0x37 reserved */
    .long   LINUX_PE_MAGIC
    .long   pe_header - _head   /* Offset to the PE header */
pe_header:
    __EFI_PE_HEADER
ENTRY(kernel_asize)
    .long _kernel_asize
ENDPROC(kernel_asize)
ENTRY(kernel_fsize)
    .long _kernel_fsize
ENDPROC(kernel_fsize)
#endif
.section ".__start", "ax"
/* void start(void) */
ENTRY(_start)
/* Config direct window and set PG */
    li.d    t0, CSR_DMW0_INIT   # WUC, PLV0, 0x8000 xxxx xxxx xxxx
    csrwr   t0, LOONGARCH_CSR_DMWIN0
    li.d    t0, CSR_DMW1_INIT   # CAC, PLV0, 0x9000 xxxx xxxx xxxx
    csrwr   t0, LOONGARCH_CSR_DMWIN1
    li.d    t0, CSR_DMW2_INIT   # WUC, PLV0, 0xa000 xxxx xxxx xxxx
    csrwr   t0, LOONGARCH_CSR_DMWIN2
    li.d    t0, CSR_DMW3_INIT   # 0x0, unused
    csrwr   t0, LOONGARCH_CSR_DMWIN3
    /* Jump to the runtime virtual address. */
    li.d    t0, CSR_DMW1_BASE
    pcaddi  t1, 0
    bstrins.d  t0, t1, (DMW_PABITS - 1), 0
    jirl    zero, t0, 0xc
    /* Enable PG */
    li.w        t0, 0xb0        # PLV=0, IE=0, PG=1
    csrwr       t0, LOONGARCH_CSR_CRMD
    li.w        t0, 0x04        # PLV=0, PIE=1, PWE=0
    csrwr       t0, LOONGARCH_CSR_PRMD
    li.w        t0, 0x00        # FPE=0, SXE=0, ASXE=0, BTE=0
    csrwr       t0, LOONGARCH_CSR_EUEN
    la.pcrel    t0, __bss_start     # clear .bss
    st.d        zero, t0, 0
    la.pcrel    t1, __bss_end__ - LONGSIZE
1:
    addi.d      t0, t0, LONGSIZE
    st.d        zero, t0, 0
    bne     t0, t1, 1b
#ifdef CONFIG_PAGE_SIZE_4KB
    li.d        t0, 0
    li.d        t1, CSR_STFILL
    csrxchg     t0, t1, LOONGARCH_CSR_IMPCTL1
#endif
    /* clear BIOS the status of timer and disable interrupts */
    li.w    t0, CSR_TINTCLR_TI
    csrwr   t0, LOONGARCH_CSR_TINTCLR
    csrwr   $r0,LOONGARCH_CSR_TCFG
    /* close BIOS timer interrupts and other interrupts */
    csrwr   $r0, LOONGARCH_CSR_ECFG
    /* enable all extern ins funcitons  */
    li.w    t0, CSR_EUEN_FPEN|CSR_EUEN_LSXEN|CSR_EUEN_LASXEN|CSR_EUEN_LBTEN
    csrwr   t0, LOONGARCH_CSR_EUEN
    /* KSave3 used for percpu base, initialized as 0 */
    csrwr       zero, PERCPU_BASE_KS
    /* GPR21 used for percpu base (runtime), initialized as 0 */
    move        u0, zero
    /* save syscall stack */
    la.pcrel    tp, syscall_stack
    PTR_LI      sp, (SYSCALL_STACK_SIZE - 0)
    PTR_ADD     sp, sp, tp
    /* 保证栈地址是16字节对齐的 */
    PTR_SRL     sp,sp,4 /* srli.d  sp,sp,4 */
    PTR_SLL     sp,sp,4 /* slli.d  sp,sp,4 */
    set_saved_sp   sp, t0, t1
    /* initialize stack pointer */
    la.pcrel    tp, init_stack
    /* Set the SP after an empty pt_regs.  */
    PTR_LI      sp, (STACK_SIZE - PT_SIZE)
    PTR_ADD     sp, sp, tp
    /* 保证栈地址是16字节对齐的 */
    PTR_SRL     sp,sp,4 /* srli.d  sp,sp,4 */
    PTR_SLL     sp,sp,4 /* slli.d  sp,sp,4 */
move a0, sp
    bl  start_kernel            /* never returns - starts up kernel */
ENDPROC(_start)
#ifdef CONFIG_SMP
/*
 * SMP slave cpus entry point.  Board specific code for bootstrap calls this
 * function after setting up the stack and tp registers.
 */
ENTRY(_start_ap)
/* Config direct window and set PG */
    li.d    t0, CSR_DMW0_INIT   # WUC, PLV0, 0x8000 xxxx xxxx xxxx
    csrwr   t0, LOONGARCH_CSR_DMWIN0
    li.d    t0, CSR_DMW1_INIT   # CAC, PLV0, 0x9000 xxxx xxxx xxxx
    csrwr   t0, LOONGARCH_CSR_DMWIN1
    li.d    t0, CSR_DMW2_INIT   # WUC, PLV0, 0xa000 xxxx xxxx xxxx
    csrwr   t0, LOONGARCH_CSR_DMWIN2
    li.d    t0, CSR_DMW3_INIT   # 0x0, unused
    csrwr   t0, LOONGARCH_CSR_DMWIN3
    /* Jump to the runtime virtual address. */
    li.d    t0, CSR_DMW1_BASE
    pcaddi  t1, 0
    bstrins.d  t0, t1, (DMW_PABITS - 1), 0
    jirl    zero, t0, 0xc
#ifdef CONFIG_PAGE_SIZE_4KB
    li.d        t0, 0
    li.d        t1, CSR_STFILL
    csrxchg     t0, t1, LOONGARCH_CSR_IMPCTL1
#endif
    /* Enable PG */
    li.w        t0, 0xb0        # PLV=0, IE=0, PG=1
    csrwr       t0, LOONGARCH_CSR_CRMD
    li.w        t0, 0x04        # PLV=0, PIE=1, PWE=0
    csrwr       t0, LOONGARCH_CSR_PRMD
    li.w        t0, 0x00        # FPE=0, SXE=0, ASXE=0, BTE=0
    csrwr       t0, LOONGARCH_CSR_EUEN
    /* clear BIOS the status of timer and disable interrupts */
    li.w    t0, CSR_TINTCLR_TI
    csrwr   t0, LOONGARCH_CSR_TINTCLR
    csrwr   $r0,LOONGARCH_CSR_TCFG
    /* close BIOS timer interrupts and other interrupts */
    csrwr   $r0, LOONGARCH_CSR_ECFG
    /* enable all extern ins funcitons  */
    li.w    t0, CSR_EUEN_FPEN|CSR_EUEN_LSXEN|CSR_EUEN_LASXEN|CSR_EUEN_LBTEN
    csrwr   t0, LOONGARCH_CSR_EUEN
   /* save syscall exception */
    csrrd       t0,LOONGARCH_CSR_CPUID
    la.pcrel    t1,syscall_stack
    li.w        t2,SYSCALL_STACK_SIZE
    mul.w       t2,t2,t0
    PTR_ADD     t1,t1,t2
    PTR_LI      sp, (SYSCALL_STACK_SIZE - 0)
    PTR_ADD     sp, sp, t1
    /* 保证栈地址是16字节对齐的 */
    PTR_SRL     sp,sp,4 /* srli.d  sp,sp,4 */
    PTR_SLL     sp,sp,4 /* slli.d  sp,sp,4 */
    la.pcrel     t1, __per_cpu_offset
    li.w         t2, __SIZEOF_POINTER__
    mul.w        t2, t2, t0
    PTR_ADD      t1, t1, t2
    PTR_L        u0, t1,0
    set_saved_sp sp, t0,t1
    /* set ap stack */
    csrrd       t0,LOONGARCH_CSR_CPUID
    la.pcrel    t1,init_stack
    li.w        t2,STACK_SIZE
    mul.w       t2,t2,t0
    PTR_ADD     t1,t1,t2
    PTR_LI      sp, (STACK_SIZE - PT_SIZE)
    PTR_ADD     sp, sp, t1
    /* 保证栈地址是16字节对齐的 */
    PTR_SRL     sp,sp,4 /* srli.d  sp,sp,4 */
    PTR_SLL     sp,sp,4 /* slli.d  sp,sp,4 */
    move      a0,sp
    //la_abs        t0,show_reg_info
    //jr          t0
    bl      start_ap
ENDPROC(_start_ap)
#endif /* CONFIG_SMP */
.section ".data"
.align 8
init_stack:
    .space STACK_SIZE * CONFIG_MAX_CPUS
syscall_stack:
    .space SYSCALL_STACK_SIZE * CONFIG_MAX_CPUS
