/**
 * @file    exception.S
 * @brief   LoongArch64 异常处理入口
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 本文件实现 LoongArch64 架构的异常和中断处理入口
 *          - 空闲等待函数 (__arch_cpu_idle)
 *          - 中断向量处理 (handle_vint)
 *          - 系统调用入口 (handle_sys)
 *          - 异常处理宏 (BUILD_HANDLER)
 *          - 地址错误处理 (handle_ade/handle_ale)
 *          - 浮点异常处理 (handle_fpe)
 *          - 扩展单元处理 (handle_fpu/handle_lsx/handle_lasx/handle_lbt)
 *          - 断点处理 (handle_bp)
 *          - 保留异常处理 (handle_reserved)
 *
 * @note MISRA-C:2012 合规
 * @warning 异常处理函数必须在禁用中断的情况下执行
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

/*************************** 修改历史 ****************************/
/*
 * 修改历史：
 * 2025-01-22    Intewell Team
 *               创建该文件
 */

/*************************** 头文件包含 ****************************/
#define ASM_USE
#include <cpu.h>
#include <asm.h>
#include <asrspace.h>

/*************************** 宏定义 ****************************/


/*************************** 类型定义 ****************************/


/*************************** 外部声明 ****************************/
    .extern handle_syscall
    .extern do_irq
    .extern do_ade
    .extern do_ale
    .extern do_bce
    .extern do_bp
    .extern do_fpe
    .extern do_fpu
    .extern do_lsx
    .extern do_lasx
    .extern do_lbt
    .extern do_ri
    .extern do_watch

/*************************** 前向声明 ****************************/


/*************************** 模块变量 ****************************/


/*************************** 函数实现 ****************************/

/**
 * @brief CPU 空闲等待函数
 *
 * @details 使 CPU 进入低功耗空闲状态
 *          执行流程：
 *          1. 启用中断（CRMD.IE = 1）
 *          2. 执行 idle 指令进入低功耗模式
 *          3. 中断唤醒后返回
 *
 * @note 此函数用于空闲任务，在无任务可运行时降低功耗
 * @note idle 指令需要中断启用才能正常工作
 * @note 如果在启用中断后、执行 idle 前发生中断，则不会进入空闲状态
 *
 * @warning 必须确保中断已启用，否则 idle 指令不会返回
 */
    .align  5
ENTRY(__arch_cpu_idle)
    /* 启用中断 */
    ori     t0, zero, CSR_CRMD_IE
    csrxchg t0, t0, LOONGARCH_CSR_CRMD

    /*
     * 如果在启用中断后、执行 idle 前发生中断，
     * 则必须确保不会进入空闲状态，
     * 因为中断可能设置了 TIF_NEED_RESCHED 或需要重新编程定时器
     */
    idle    0

1:
    jr      ra
ENDPROC(__arch_cpu_idle)

/**
 * @brief 中断向量处理函数
 *
 * @details 处理所有硬件中断
 *          执行流程：
 *          1. 保存所有寄存器（SAVE_ALL）
 *          2. 检查是否从空闲等待中断区域返回
 *          3. 调用 C 语言中断处理函数 do_irq
 *          4. 恢复所有寄存器并返回（RESTORE_ALL_AND_RET）
 *
 * @note 此函数处理所有硬件中断（包括定时器、IPI、设备中断等）
 * @note 如果从空闲中断区域返回，需要修正 ERA 寄存器
 * @note 空闲中断区域为 __arch_cpu_idle 函数中的 idle 指令及其前后的 3 条指令
 *
 * @param a0 异常上下文指针（sp）
 * @param a1 异常上下文指针（sp）
 *
 * @return 此函数不会返回到调用点，而是返回到被中断的代码
 */
ENTRY(handle_vint)
    SAVE_ALL

    /* 获取空闲中断区域后的返回地址 */
    la_abs  t1, 1b
    LONG_L  t0, sp, PT_ERA

    /* 检查是否从空闲中断区域返回（3 条指令区域） */
    ori     t0, t0, 0b1100
    bne     t0, t1, 1f
    LONG_S  t0, sp, PT_ERA

1:
    /* 调用 C 语言中断处理函数 */
    move    a0, sp
    move    a1, sp
    la_abs  t0, do_irq
    jirl    ra, t0, 0

    /* 恢复上下文并返回 */
    RESTORE_ALL_AND_RET
ENDPROC(handle_vint)

/**
 * @brief 构建异常出错地址准备宏
 *
 * @details 读取 BADV 寄存器（出错地址）并保存到异常上下文
 *
 * @note 用于地址错误异常（ADE、ALE）
 */
    .macro  build_prep_badv
    csrrd   t0, LOONGARCH_CSR_BADV
    PTR_S   t0, sp, PT_BVADDR
    .endm

/**
 * @brief 构建浮点状态准备宏
 *
 * @details 读取 FCSR 寄存器（浮点控制和状态寄存器）到 a1
 *
 * @note 用于浮点异常（FPE）
 */
    .macro  build_prep_fcsr
    movfcsr2gr  a1, fcsr0
    .endm

/**
 * @brief 空准备宏
 *
 * @details 不执行任何操作
 *
 * @note 用于不需要额外准备的异常
 */
    .macro  build_prep_none
    .endm

/**
 * @brief 构建异常处理函数宏
 *
 * @details 生成标准异常处理函数
 *          执行流程：
 *          1. 保存所有寄存器（SAVE_ALL）
 *          2. 执行准备操作（build_prep_*）
 *          3. 调用 C 语言异常处理函数 do_<handler>
 *          4. 恢复所有寄存器并返回（RESTORE_ALL_AND_RET）
 *
 * @param exception 异常名称
 * @param handler   C 语言处理函数名（do_<handler>）
 * @param prep      准备操作（badv/fcsr/none）
 *
 * @note 此宏为所有异常生成标准化的处理函数
 * @note 异常上下文指针通过 a0 传递给 C 函数
 */
    .macro  BUILD_HANDLER exception handler prep
    .align  5
ENTRY(handle_\exception)
666:
    SAVE_ALL
    build_prep_\prep
    move    a0, sp
    la_abs  t0, do_\handler
    jirl    ra, t0, 0
668:
    RESTORE_ALL_AND_RET
ENDPROC(handle_\exception)
    .pushsection ".data", "aw", %progbits
    .popsection
    .endm

/**
 * @brief 地址错误异常处理（未对齐）
 *
 * @details 处理未对齐的内存访问异常
 *          保存出错地址并调用 do_ale 处理
 *
 * @note ALE（Alignment Error）异常
 */
BUILD_HANDLER ale ale badv

/**
 * @brief 边界检查异常处理
 *
 * @details 处理边界检查异常
 *          调用 do_bce 处理
 *
 * @note BCE（Bound Check Exception）异常
 */
BUILD_HANDLER bce bce none

/**
 * @brief 断点异常处理
 *
 * @details 处理断点异常
 *          调用 do_bp 处理
 *
 * @note BP（BreakPoint）异常
 */
BUILD_HANDLER bp bp none

/**
 * @brief 浮点异常处理
 *
 * @details 处理浮点运算异常
 *          保存浮点状态寄存器并调用 do_fpe 处理
 *
 * @note FPE（Floating Point Exception）异常
 */
BUILD_HANDLER fpe fpe fcsr

/**
 * @brief 浮点单元禁用异常处理
 *
 * @details 处理浮点单元被禁用时访问浮点寄存器的异常
 *          调用 do_fpu 处理
 *
 * @note FPU（Floating Point Unit）异常
 */
BUILD_HANDLER fpu fpu none

/**
 * @brief LoongArch SIMD 扩展禁用异常处理
 *
 * @details 处理 LSX 扩展被禁用时访问 SIMD 寄存器的异常
 *          调用 do_lsx 处理
 *
 * @note LSX（LoongArch SIMD eXtension）异常
 */
BUILD_HANDLER lsx lsx none

/**
 * @brief LoongArch 高级 SIMD 扩展禁用异常处理
 *
 * @details 处理 LASX 扩展被禁用时访问高级 SIMD 寄存器的异常
 *          调用 do_lasx 处理
 *
 * @note LASX（LoongArch Advanced SIMD eXtension）异常
 */
BUILD_HANDLER lasx lasx none

/**
 * @brief LoongArch 二进制翻译禁用异常处理
 *
 * @details 处理 LBT 扩展被禁用时使用二进制翻译指令的异常
 *          调用 do_lbt 处理
 *
 * @note LBT（LoongArch Binary Translation）异常
 */
BUILD_HANDLER lbt lbt none

/**
 * @brief 保留指令异常处理
 *
 * @details 处理执行保留指令的异常
 *          调用 do_ri 处理
 *
 * @note RI（Reserved Instruction）异常
 */
BUILD_HANDLER ri ri none

/**
 * @brief 观察点异常处理
 *
 * @details 处理硬件观察点触发的异常
 *          调用 do_watch 处理
 *
 * @note Watch 异常
 */
BUILD_HANDLER watch watch none

/**
 * @brief 地址错误异常处理（地址越界）
 *
 * @details 处理地址越界访问异常
 *          保存出错地址并调用 do_ade 处理
 *
 * @note ADE（Address Error）异常
 */
BUILD_HANDLER ade ade badv

/*
 * BUILD_HANDLER reserved reserved none
 * 其他保留异常使用默认处理函数
 */

/**
 * @brief 系统调用入口（从异常向量表）
 *
 * @details 系统调用异常的入口跳转函数
 *          直接跳转到 handle_syscall 函数
 *
 * @note 此函数由异常向量表调用
 * @note 系统调用号为 EXCCODE_SYS（0xB）
 */
ENTRY(handle_sys)
    la_abs  t0, handle_syscall
    jr      t0
ENDPROC(handle_sys)

/**
 * @brief 保留异常处理函数
 *
 * @details 处理未定义或保留的异常
 *          执行流程：
 *          1. 向 0x800000001fe001e0 地址写入 0x49
 *          2. 执行 ertn 返回
 *
 * @note 此函数用于调试和日志记录
 * @note 写入操作可能用于触发硬件调试逻辑
 * @note TODO: 完善 LOONGARCH 保留异常处理
 *
 * @warning 此函数不应在生产环境中使用
 */
ENTRY(handle_reserved)
    /* TODO LOONGARCH: 需要完善保留异常处理逻辑 */
    li.d    t1, 0x800000001fe001e0
    li.w    t2, 0x49
    st.b    t2, t1, 0
    ertn
ENDPROC(handle_reserved)
