#include <asm_offsets_build.h>
#define _HARD_FLOAT_
#include <context.h>
#include <ttos.h>
int main(void)
{
    DEFINE(ARCH_CONTEXT_SIZE,       sizeof(struct arch_context));
}
static void __attribute__((__used__)) output_ptreg_defines(void)
{
    COMMENT("LoongArch arch_context offsets.");
    OFFSET(PT_R0, arch_context, regs[0]);
    OFFSET(PT_R1, arch_context, regs[1]);
    OFFSET(PT_R2, arch_context, regs[2]);
    OFFSET(PT_R3, arch_context, regs[3]);
    OFFSET(PT_R4, arch_context, regs[4]);
    OFFSET(PT_R5, arch_context, regs[5]);
    OFFSET(PT_R6, arch_context, regs[6]);
    OFFSET(PT_R7, arch_context, regs[7]);
    OFFSET(PT_R8, arch_context, regs[8]);
    OFFSET(PT_R9, arch_context, regs[9]);
    OFFSET(PT_R10, arch_context, regs[10]);
    OFFSET(PT_R11, arch_context, regs[11]);
    OFFSET(PT_R12, arch_context, regs[12]);
    OFFSET(PT_R13, arch_context, regs[13]);
    OFFSET(PT_R14, arch_context, regs[14]);
    OFFSET(PT_R15, arch_context, regs[15]);
    OFFSET(PT_R16, arch_context, regs[16]);
    OFFSET(PT_R17, arch_context, regs[17]);
    OFFSET(PT_R18, arch_context, regs[18]);
    OFFSET(PT_R19, arch_context, regs[19]);
    OFFSET(PT_R20, arch_context, regs[20]);
    OFFSET(PT_R21, arch_context, regs[21]);
    OFFSET(PT_R22, arch_context, regs[22]);
    OFFSET(PT_R23, arch_context, regs[23]);
    OFFSET(PT_R24, arch_context, regs[24]);
    OFFSET(PT_R25, arch_context, regs[25]);
    OFFSET(PT_R26, arch_context, regs[26]);
    OFFSET(PT_R27, arch_context, regs[27]);
    OFFSET(PT_R28, arch_context, regs[28]);
    OFFSET(PT_R29, arch_context, regs[29]);
    OFFSET(PT_R30, arch_context, regs[30]);
    OFFSET(PT_R31, arch_context, regs[31]);
    OFFSET(PT_CRMD, arch_context, csr_crmd);
    OFFSET(PT_PRMD, arch_context, csr_prmd);
    OFFSET(PT_EUEN, arch_context, csr_euen);
    OFFSET(PT_ECFG, arch_context, csr_ecfg);
    OFFSET(PT_ESTAT, arch_context, csr_estat);
    OFFSET(PT_ERA, arch_context, csr_era);
    OFFSET(PT_BVADDR, arch_context, csr_badvaddr);
    OFFSET(PT_ORIG_A0, arch_context, orig_a0);
    DEFINE(PT_SIZE, sizeof(struct arch_context));
}
static void __attribute__((__used__)) output_thread_fpu_defines(void)
{
    OFFSET(THREAD_FPR0, loongarch_fpu, fpr[0]);
    OFFSET(THREAD_FPR1, loongarch_fpu, fpr[1]);
    OFFSET(THREAD_FPR2, loongarch_fpu, fpr[2]);
    OFFSET(THREAD_FPR3, loongarch_fpu, fpr[3]);
    OFFSET(THREAD_FPR4, loongarch_fpu, fpr[4]);
    OFFSET(THREAD_FPR5, loongarch_fpu, fpr[5]);
    OFFSET(THREAD_FPR6, loongarch_fpu, fpr[6]);
    OFFSET(THREAD_FPR7, loongarch_fpu, fpr[7]);
    OFFSET(THREAD_FPR8, loongarch_fpu, fpr[8]);
    OFFSET(THREAD_FPR9, loongarch_fpu, fpr[9]);
    OFFSET(THREAD_FPR10, loongarch_fpu, fpr[10]);
    OFFSET(THREAD_FPR11, loongarch_fpu, fpr[11]);
    OFFSET(THREAD_FPR12, loongarch_fpu, fpr[12]);
    OFFSET(THREAD_FPR13, loongarch_fpu, fpr[13]);
    OFFSET(THREAD_FPR14, loongarch_fpu, fpr[14]);
    OFFSET(THREAD_FPR15, loongarch_fpu, fpr[15]);
    OFFSET(THREAD_FPR16, loongarch_fpu, fpr[16]);
    OFFSET(THREAD_FPR17, loongarch_fpu, fpr[17]);
    OFFSET(THREAD_FPR18, loongarch_fpu, fpr[18]);
    OFFSET(THREAD_FPR19, loongarch_fpu, fpr[19]);
    OFFSET(THREAD_FPR20, loongarch_fpu, fpr[20]);
    OFFSET(THREAD_FPR21, loongarch_fpu, fpr[21]);
    OFFSET(THREAD_FPR22, loongarch_fpu, fpr[22]);
    OFFSET(THREAD_FPR23, loongarch_fpu, fpr[23]);
    OFFSET(THREAD_FPR24, loongarch_fpu, fpr[24]);
    OFFSET(THREAD_FPR25, loongarch_fpu, fpr[25]);
    OFFSET(THREAD_FPR26, loongarch_fpu, fpr[26]);
    OFFSET(THREAD_FPR27, loongarch_fpu, fpr[27]);
    OFFSET(THREAD_FPR28, loongarch_fpu, fpr[28]);
    OFFSET(THREAD_FPR29, loongarch_fpu, fpr[29]);
    OFFSET(THREAD_FPR30, loongarch_fpu, fpr[30]);
    OFFSET(THREAD_FPR31, loongarch_fpu, fpr[31]);
    OFFSET(THREAD_FCSR, loongarch_fpu, fcsr);
    OFFSET(THREAD_FCC,  loongarch_fpu, fcc);
    OFFSET(THREAD_FTOP, loongarch_fpu, ftop);
}
static void __attribute__((__used__)) mm_defines(void)
{
    DEFINE(_PTE_T_LOG2, PTE_T_LOG2);
}
static void __attribute__((__used__)) output_thread_info_defines(void)
{
    COMMENT("LoongArch thread_info offsets.");
    DEFINE(_THREAD_SIZE, THREAD_SIZE);
    DEFINE(_THREAD_MASK, THREAD_MASK);
    OFFSET(THREAD_BADV,task_context,csr_badvaddr);
    OFFSET(THREAD_CRMD,task_context,csr_crmd);
    OFFSET(THREAD_PRMD,task_context,csr_prmd);
    OFFSET(THREAD_EUEN,task_context,csr_euen);
    OFFSET(THREAD_ECFG,task_context,csr_ecfg);
    OFFSET(THREAD_ESTAT,task_context,csr_estat);
    OFFSET(THREAD_PC,task_context,pc);
    OFFSET(THREAD_SP,task_context,sp);
    OFFSET(THREAD_TLS,task_context,tls);
    OFFSET(THREAD_PGDL,task_context,pgdl);
    OFFSET(THREAD_ASID,task_context,asid);
    OFFSET(THREAD_VFLAG,task_context,vintflag);
    OFFSET(THREAD_REG0,task_context,regs[0]);
    OFFSET(THREAD_REG1,task_context,regs[1]);
    OFFSET(THREAD_REG2,task_context,regs[2]);
    OFFSET(THREAD_REG3,task_context,regs[3]);
    OFFSET(THREAD_REG4,task_context,regs[4]);
    OFFSET(THREAD_REG5,task_context,regs[5]);
    OFFSET(THREAD_REG6,task_context,regs[6]);
    OFFSET(THREAD_REG7,task_context,regs[7]);
    OFFSET(THREAD_REG8,task_context,regs[8]);
    OFFSET(THREAD_REG9,task_context,regs[9]);
    OFFSET(THREAD_REG10,task_context,regs[10]);
    OFFSET(THREAD_REG11,task_context,regs[11]);
    OFFSET(THREAD_REG12,task_context,regs[12]);
    OFFSET(THREAD_REG13,task_context,regs[13]);
    OFFSET(THREAD_REG14,task_context,regs[14]);
    OFFSET(THREAD_REG15,task_context,regs[15]);
    OFFSET(THREAD_REG16,task_context,regs[16]);
    OFFSET(THREAD_REG17,task_context,regs[17]);
    OFFSET(THREAD_REG18,task_context,regs[18]);
    OFFSET(THREAD_REG19,task_context,regs[19]);
    OFFSET(THREAD_REG20,task_context,regs[20]);
    OFFSET(THREAD_REG21,task_context,regs[21]);
    OFFSET(THREAD_REG22,task_context,regs[22]);
    OFFSET(THREAD_REG23,task_context,regs[23]);
    OFFSET(THREAD_REG24,task_context,regs[24]);
    OFFSET(THREAD_REG25,task_context,regs[25]);
    OFFSET(THREAD_REG26,task_context,regs[26]);
    OFFSET(THREAD_REG27,task_context,regs[27]);
    OFFSET(THREAD_REG28,task_context,regs[28]);
    OFFSET(THREAD_REG29,task_context,regs[29]);
    OFFSET(THREAD_REG30,task_context,regs[30]);
    OFFSET(THREAD_REG31,task_context,regs[31]);
#ifdef _HARD_FLOAT_
    OFFSET(THREAD_FPU,task_context,fpu);
#if 0
    OFFSET(THREAD_FPU_FCC,task_context,fpu.fcc);
    OFFSET(THREAD_FPU_FCSR,task_context,fpu.fcsr);
    OFFSET(THREAD_FPU_FTOP,task_context,fpu.ftop);
    OFFSET(THREAD_FPU_FPR0, loongarch_fpu, fpu.fpr[0]);
    OFFSET(THREAD_FPU_FPR1, loongarch_fpu, fpu.fpr[1]);
    OFFSET(THREAD_FPU_FPR2, loongarch_fpu, fpu.fpr[2]);
    OFFSET(THREAD_FPU_FPR3, loongarch_fpu, fpu.fpr[3]);
    OFFSET(THREAD_FPU_FPR4, loongarch_fpu, fpu.fpr[4]);
    OFFSET(THREAD_FPU_FPR5, loongarch_fpu, fpu.fpr[5]);
    OFFSET(THREAD_FPU_FPR6, loongarch_fpu, fpu.fpr[6]);
    OFFSET(THREAD_FPU_FPR7, loongarch_fpu, fpu.fpr[7]);
    OFFSET(THREAD_FPU_FPR8, loongarch_fpu, fpu.fpr[8]);
    OFFSET(THREAD_FPU_FPR9, loongarch_fpu, fpu.fpr[9]);
    OFFSET(THREAD_FPU_FPR10, loongarch_fpu, fpu.fpr[10]);
    OFFSET(THREAD_FPU_FPR11, loongarch_fpu, fpu.fpr[11]);
    OFFSET(THREAD_FPU_FPR12, loongarch_fpu, fpu.fpr[12]);
    OFFSET(THREAD_FPU_FPR13, loongarch_fpu, fpu.fpr[13]);
    OFFSET(THREAD_FPU_FPR14, loongarch_fpu, fpu.fpr[14]);
    OFFSET(THREAD_FPU_FPR15, loongarch_fpu, fpu.fpr[15]);
    OFFSET(THREAD_FPU_FPR16, loongarch_fpu, fpu.fpr[16]);
    OFFSET(THREAD_FPU_FPR17, loongarch_fpu, fpu.fpr[17]);
    OFFSET(THREAD_FPU_FPR18, loongarch_fpu, fpu.fpr[18]);
    OFFSET(THREAD_FPU_FPR19, loongarch_fpu, fpu.fpr[19]);
    OFFSET(THREAD_FPU_FPR20, loongarch_fpu, fpu.fpr[20]);
    OFFSET(THREAD_FPU_FPR21, loongarch_fpu, fpu.fpr[21]);
    OFFSET(THREAD_FPU_FPR22, loongarch_fpu, fpu.fpr[22]);
    OFFSET(THREAD_FPU_FPR23, loongarch_fpu, fpu.fpr[23]);
    OFFSET(THREAD_FPU_FPR24, loongarch_fpu, fpu.fpr[24]);
    OFFSET(THREAD_FPU_FPR25, loongarch_fpu, fpu.fpr[25]);
    OFFSET(THREAD_FPU_FPR26, loongarch_fpu, fpu.fpr[26]);
    OFFSET(THREAD_FPU_FPR27, loongarch_fpu, fpu.fpr[27]);
    OFFSET(THREAD_FPU_FPR28, loongarch_fpu, fpu.fpr[28]);
    OFFSET(THREAD_FPU_FPR29, loongarch_fpu, fpu.fpr[29]);
    OFFSET(THREAD_FPU_FPR30, loongarch_fpu, fpu.fpr[30]);
    OFFSET(THREAD_FPU_FPR31, loongarch_fpu, fpu.fpr[31]);
#endif
#endif
}
