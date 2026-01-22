#define ASM_USE
#include <asm.h>
#include <system/const.h>
#include <cpu.h>
#include <context.h>
.global return2user
return2user:
    move        a0,zero                  /* 清空参数寄存器 a0 */
    move        sp,a1                    /* 设置用户栈指针 */
    csrwr       a2,LOONGARCH_CSR_ERA    /* 设置用户入口地址 */
    csrwr       a3,LOONGARCH_CSR_CRMD   /* 设置当前模式寄存器 CRMD */
    move        a1,zero
    move        a2,zero
    move        a3,zero
    move        a4,zero
    move        a5,zero
    move        a6,zero
    move        a7,zero
    move        t0,zero
    move        t1,zero
    move        t2,zero
    move        t3,zero
    move        t4,zero
    move        t5,zero
    move        t6,zero
    move        t7,zero
    move        t8,zero
    move        s0,zero
    move        s1,zero
    move        s2,zero
    move        s3,zero
    move        s4,zero
    move        s5,zero
    move        s6,zero
    move        s7,zero
    move        s8,zero
    ibar        0
    dbar        0
    ertn
