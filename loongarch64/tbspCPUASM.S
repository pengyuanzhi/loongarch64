/**
 * @file    tbspCPUASM.S
 * @brief   LoongArch64 板级支持包汇编函数
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 本文件实现 LoongArch64 架构的板级支持包（BSP）汇编函数
 *          - IDLE 任务入口加载
 *          - 优先级位图最高有效位查找
 *          - 任务栈指针加载
 *          - 任务上下文保存和恢复
 *          - FPU 状态初始化
 *
 * @note 这些函数是任务调度的核心底层实现
 * @note 所有函数必须保证栈 16 字节对齐
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

/*************************** 修改历史 ****************************/
/*
 * 修改历史：
 * 2024-08-28    科东（广州）软件科技有限公司
 *               创建文件
 * 2025-01-22    Intewell Team
 *               按照最新代码风格规范重构
 */

/*************************** 头文件包含 ****************************/
#define ASM_USE
#include <cpu.h>
#include <asm.h>
#include <ttosHal.h>

/*************************** 宏定义 ****************************/


/*************************** 类型定义 ****************************/


/*************************** 外部声明 ****************************/


/*************************** 前向声明 ****************************/


/*************************** 模块变量 ****************************/


/*************************** 函数实现 ****************************/

/**
 * @brief 进入 IDLE 任务
 *
 * @details 设置 IDLE 任务的栈并跳转到 IDLE 任务入口
 *          执行流程：
 *          1. 预留 32 字节安全空间
 *          2. 确保 16 字节对齐
 *          3. 调用 ttosIdleTaskEntry 函数
 *          4. 进入无限循环
 *
 * @param a0 stackTop  IDLE 任务栈顶指针
 *
 * @return 此函数不会返回
 *
 * @note IDLE 任务是系统空闲时执行的任务
 * @note 栈必须 16 字节对齐（LoongArch64 ABI 要求）
 */
ENTRY(idleTaskEntryLoad)
    BACKUP_T0T1
    li.d    t0, 32
    sub.d   a0, a0, t0       /* 预留 32 字节安全空间 */
    srli.d  a0, a0, 4        /* 右移 4 位 */
    slli.d  a0, a0, 4        /* 左移 4 位，确保 16 字节对齐 */
    move    sp, a0
    RELOAD_T0T1

    /*
     * 调用 IDLE 任务入口函数
     * 该函数不会返回
     */
    bl      ttosIdleTaskEntry

    /*
     * 如果 IDLE 任务返回，进入无限循环
     * 正常情况下不应执行到这里
     */
1:
    b       1b
ENDPROC(idleTaskEntryLoad)

/**
 * @brief 获取优先级位图中最高有效位
 *
 * @details 计算优先级位图中从最高有效位（bit0）开始第一个为 1 的位所在位数
 *
 * @param a0 priBitMap  优先级位图（64 位无符号整数）
 *
 * @return a0 最高有效位的位置（0-63）
 *
 * @note 使用 CLZ（Count Leading Zeros）指令
 * @note CLZ 返回前导零的数量，最高有效位位置 = 63 - CLZ
 *
 * @par 示例
 * @code
 * uint64_t bitmap = 0x00010000U;  // bit 16 为 1
 * uint32_t pos = tbspGetMSB(bitmap);  // 返回 16
 * @endcode
 */
.text
.align  4
.global tbspGetMSB
tbspGetMSB:
    clz.w   a0, a0             /* 计算前导零数量 */
    jirl    zero, ra, 0        /* 返回 */

/**
 * @brief 加载任务栈指针
 *
 * @details 设置当前任务的栈指针
 *          执行流程：
 *          1. 预留 32 字节安全空间
 *          2. 确保 16 字节对齐
 *          3. 设置栈指针寄存器 sp
 *
 * @param a0 stackTop  任务栈顶指针
 *
 * @return 无
 *
 * @note 此函数用于任务切换时设置新任务的栈
 * @note 栈必须 16 字节对齐（LoongArch64 ABI 要求）
 */
.text
.align  4
.global tbspLoadStack
tbspLoadStack:
    BACKUP_T0T1
    li.d    t0, 32
    sub.d   a0, a0, t0         /* 预留 32 字节安全空间 */
    srli.d  a0, a0, 4          /* 右移 4 位 */
    slli.d  a0, a0, 4          /* 左移 4 位，确保 16 字节对齐 */
    move    sp, a0             /* 设置栈指针 */
    RELOAD_T0T1
    jirl    zero, ra, 0        /* 返回 */

/**
 * @brief 保存任务上下文
 *
 * @details 保存当前任务的运行上下文到任务控制块
 *          执行流程：
 *          1. 保存通用寄存器（task_context_save 宏）
 *          2. 保存 CSR 寄存器（CRMD、PRMD、EUEN、ECFG、ESTAT、BADV）
 *          3. 保存 TLS 指针（tp）
 *          4. 保存页表基址（PGDL）和地址空间 ID（ASID）
 *          5. 保存程序计数器（ERA）和栈指针（SP）
 *          6. 保存虚拟中断标志（VFLAG）
 *          7. 保存 FPU 状态（如果启用）
 *
 * @param a0 context  任务上下文指针（T_TBSP_TaskContext *）
 *
 * @return a0 0（表示上下文已保存）
 *
 * @note 此函数在任务切换前调用
 * @note 调用 task_context_save 宏保存通用寄存器
 * @note 使用 BACKUP_T0T1 和 RELOAD_T0T1 保护 t0 和 t1 寄存器
 */
.text
.align  4
.global tbspSaveTaskContext
tbspSaveTaskContext:
    task_context_save a0      /* 保存通用寄存器 */
    BACKUP_T0T1

    /* 保存 CSR 寄存器 */
    csrrd   t0, LOONGARCH_CSR_CRMD
    st.d    t0, a0, THREAD_CRMD
    csrrd   t0, LOONGARCH_CSR_PRMD
    st.d    t0, a0, THREAD_PRMD
    csrrd   t0, LOONGARCH_CSR_EUEN
    st.d    t0, a0, THREAD_EUEN
    csrrd   t0, LOONGARCH_CSR_ECFG
    st.d    t0, a0, THREAD_ECFG
    csrrd   t0, LOONGARCH_CSR_ESTAT
    st.d    t0, a0, THREAD_ESTAT
    csrrd   t0, LOONGARCH_CSR_BADV
    st.d    t0, a0, THREAD_BADV

    /* 保存 TLS 指针 */
    st.d    tp, a0, THREAD_TLS

    /* 保存页表基址和地址空间 ID */
    csrrd   t0, LOONGARCH_CSR_PGDL
    st.d    t0, a0, THREAD_PGDL
    csrrd   t0, LOONGARCH_CSR_ASID
    st.d    t0, a0, THREAD_ASID

    /* 保存程序计数器和栈指针 */
    csrrd   t0, LOONGARCH_CSR_ERA
    st.d    t0, a0, THREAD_PC
    st.d    sp, a0, THREAD_SP

    /* 保存虚拟中断标志 */
    ld.d    t0, a0, THREAD_VFLAG
    csrwr   t0, LOONGARCH_CSR_KS4

#ifdef _HARD_FLOAT_
    /* 保存 FPU 状态 */
    addi.d  a0, a0, THREAD_FPU
    fpu_save_csr    a0 t0
    fpu_save_double a0 t1
    fpu_save_cc     a0 t0 t1
#endif /* _HARD_FLOAT_ */

    RELOAD_T0T1
    li.w    a0, 0              /* 返回 0 */
    jirl    zero, ra, 0

/**
 * @brief 恢复任务上下文
 *
 * @details 从任务控制块恢复任务的运行上下文
 *          执行流程：
 *          1. 恢复 CSR 寄存器（CRMD、PRMD、EUEN、ECFG、ESTAT、BADV）
 *          2. 恢复 TLS 指针（tp）
 *          3. 恢复页表基址（PGDL）和地址空间 ID（ASID）
 *          4. 恢复程序计数器（ERA）和栈指针（SP）
 *          5. 恢复虚拟中断标志（VFLAG）
 *          6. 恢复 FPU 状态（如果启用）
 *          7. 恢复通用寄存器（task_context_restore 宏）
 *
 * @param a0 context  任务上下文指针（T_TBSP_TaskContext *）
 *
 * @return a0 1（表示上下文已恢复）
 *
 * @note 此函数在任务切换后调用
 * @note 调用 task_context_restore 宏恢复通用寄存器
 * @note 使用 BACKUP_T0T1 和 RELOAD_T0T1 保护 t0 和 t1 寄存器
 */
.text
.align  4
.global tbspRestoreTaskContext
tbspRestoreTaskContext:
    task_context_restore a0   /* 恢复通用寄存器 */
    BACKUP_T0T1

    /* 恢复 CSR 寄存器 */
    ld.d    t0, a0, THREAD_CRMD
    csrwr   t0, LOONGARCH_CSR_CRMD
    ld.d    t0, a0, THREAD_PRMD
    csrwr   t0, LOONGARCH_CSR_PRMD
    ld.d    t0, a0, THREAD_EUEN
    csrwr   t0, LOONGARCH_CSR_EUEN
    ld.d    t0, a0, THREAD_ECFG
    csrwr   t0, LOONGARCH_CSR_ECFG
    ld.d    t0, a0, THREAD_ESTAT
    csrwr   t0, LOONGARCH_CSR_ESTAT
    ld.d    t0, a0, THREAD_BADV
    csrwr   t0, LOONGARCH_CSR_BADV

    /* 恢复 TLS 指针 */
    ld.d    tp, a0, THREAD_TLS

    /* 恢复页表基址和地址空间 ID */
    ld.d    t0, a0, THREAD_PGDL
    csrwr   t0, LOONGARCH_CSR_PGDL
    ld.d    t0, a0, THREAD_ASID
    csrwr   t0, LOONGARCH_CSR_ASID

    /* 恢复程序计数器和栈指针 */
    ld.d    t0, a0, THREAD_PC
    csrwr   t0, LOONGARCH_CSR_ERA
    ld.d    sp, a0, THREAD_SP

    /* 恢复虚拟中断标志 */
    csrrd   t0, LOONGARCH_CSR_KS4
    st.d    t0, a0, THREAD_VFLAG

#ifdef _HARD_FLOAT_
    /* 恢复 FPU 状态 */
    addi.d  a0, a0, THREAD_FPU
    fpu_restore_double  a0 t0
    fpu_restore_csr     a0 t0 t1
    fpu_restore_cc      a0 t0 t1
#endif /* _HARD_FLOAT_ */

    RELOAD_T0T1
    li.w    a0, 1              /* 返回 1 */
    jirl    zero, ra, 0

#ifdef _HARD_FLOAT_
/**
 * @brief 初始化 FPU 状态和控制寄存器
 *
 * @details 初始化浮点单元的状态和控制寄存器
 *
 * @return 无
 *
 * @note 此函数尚未实现
 * @note TODO: 需要根据实际需求实现 FPU 初始化逻辑
 */
.text
.align  4
.global fpscrAsmInit
fpscrAsmInit:
    /* TODO: 未实现 FPU 初始化 */
    jirl    zero, ra, 0
#endif /* _HARD_FLOAT_ */
