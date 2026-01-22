/**
 * @file    crt1.S
 * @brief   LoongArch64 C运行时启动代码
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 本文件实现 LoongArch64 架构的 C 运行时启动代码
 *          - 主核启动入口 (_start)
 *          - 从核启动入口 (_start_ap)
 *          - 直接映射窗口配置
 *          - MMU 初始化和启用
 *          - 栈初始化
 *          - BSS 段清零
 *          - 外部扩展单元使能（FPU、LSX、LASX、LBT）
 *
 * @note MISRA-C:2012 合规
 * @warning 此函数执行后不会返回
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
#include <asm.h>
#include <cpu.h>

/*************************** 宏定义 ****************************/
#define STACK_SIZE         0x10000
#define SYSCALL_STACK_SIZE 0x30000

/*************************** 类型定义 ****************************/


/*************************** 外部声明 ****************************/


/*************************** 前向声明 ****************************/


/*************************** 模块变量 ****************************/


/*************************** 函数实现 ****************************/

#ifdef CONFIG_EFI_STUB
#include "efi-header.S"

/**
 * @brief EFI 启动头
 *
 * @details 用于 EFI 启动的 PE 头和镜像头
 */
.section    ".head.text","ax"
_head:
    .word   MZ_MAGIC        /* "MZ", MS-DOS header */
    .org    0x8
    .dword  _start          /* Kernel entry point (physical address) */
    .dword  _kernel_asize   /* Kernel image effective size */
    .quad   PHYS_LINK_KADDR /* Kernel image load offset from start of RAM */
    .org    0x38            /* 0x20 ~ 0x37 reserved */
    .long   LINUX_PE_MAGIC
    .long   pe_header - _head /* Offset to the PE header */

pe_header:
    __EFI_PE_HEADER

/**
 * @brief 内核镜像大小
 */
ENTRY(kernel_asize)
    .long _kernel_asize
ENDPROC(kernel_asize)

/**
 * @brief 内核文件大小
 */
ENTRY(kernel_fsize)
    .long _kernel_fsize
ENDPROC(kernel_fsize)
#endif /* CONFIG_EFI_STUB */

/**
 * @brief 主核启动入口
 *
 * @details LoongArch64 系统主核启动入口点
 *          执行流程：
 *          1. 配置直接映射窗口（DMW0-DMW3）
 *          2. 跳转到运行时虚拟地址
 *          3. 启用 MMU（分页机制）
 *          4. 清零 BSS 段
 *          5. 配置页大小（4KB 模式）
 *          6. 清除 BIOS 定时器状态并禁用中断
 *          7. 使能外部扩展单元（FPU、LSX、LASX、LBT）
 *          8. 初始化 per-CPU 基址
 *          9. 设置系统调用栈
 *          10. 设置初始化栈
 *          11. 跳转到 C 内核入口点
 *
 * @note 此函数是系统的第一个执行点，由硬件直接调用
 * @note 执行环境：物理地址模式，MMU 未启用
 * @note 执行后：虚拟地址模式，MMU 已启用
 *
 * @warning 此函数执行后不会返回
 */
.section ".__start", "ax"
ENTRY(_start)

    /*
     * 配置直接映射窗口（Direct Mapping Window）
     * DMW 允许虚拟地址直接映射到物理地址，无需页表转换
     *
     * DMW0: WUC (Uncachable), PLV0, 0x8000 xxxx xxxx xxxx
     * DMW1: CAC (Cacheable), PLV0, 0x9000 xxxx xxxx xxxx
     * DMW2: WUC (Uncachable), PLV0, 0xa000 xxxx xxxx xxxx
     * DMW3: 未使用
     */
    li.d    t0, CSR_DMW0_INIT
    csrwr   t0, LOONGARCH_CSR_DMWIN0
    li.d    t0, CSR_DMW1_INIT
    csrwr   t0, LOONGARCH_CSR_DMWIN1
    li.d    t0, CSR_DMW2_INIT
    csrwr   t0, LOONGARCH_CSR_DMWIN2
    li.d    t0, CSR_DMW3_INIT
    csrwr   t0, LOONGARCH_CSR_DMWIN3

    /*
     * 跳转到运行时虚拟地址
     *
     * 使用 DMW1 的基址（0x9000 xxxx xxxx xxxx）
     * 计算当前 PC 的物理地址，然后插入到 DMW1 基址中
     * 最后通过 jirl 指令跳转到虚拟地址
     */
    li.d    t0, CSR_DMW1_BASE
    pcaddi  t1, 0
    bstrins.d  t0, t1, (DMW_PABITS - 1), 0
    jirl    zero, t0, 0xc

    /*
     * 启用 MMU（分页机制）
     *
     * CRMD 寄存器配置：
     *   - PLV = 0 (特权级 0，内核模式)
     *   - IE = 0 (中断禁用)
     *   - PG = 1 (分页使能)
     *
     * PRMD 寄存器配置：
     *   - PPLV = 0 (前一个特权级 0)
     *   - PIE = 1 (前一个中断使能)
     *   - PWE = 0 (前一个等待禁用)
     *
     * EUEN 寄存器配置：
     *   - FPE = 0 (FPU 禁用)
     *   - SXE = 0 (SIMD 扩展禁用)
     *   - ASXE = 0 (高级 SIMD 扩展禁用)
     *   - BTE = 0 (二进制翻译禁用)
     */
    li.w    t0, 0xb0        /* PLV=0, IE=0, PG=1 */
    csrwr   t0, LOONGARCH_CSR_CRMD
    li.w    t0, 0x04        /* PLV=0, PIE=1, PWE=0 */
    csrwr   t0, LOONGARCH_CSR_PRMD
    li.w    t0, 0x00        /* FPE=0, SXE=0, ASXE=0, BTE=0 */
    csrwr   t0, LOONGARCH_CSR_EUEN

    /*
     * 清零 BSS 段
     *
     * BSS 段包含未初始化的全局变量和静态变量
     * 必须在 C 代码执行前清零
     */
    la.pcrel    t0, __bss_start
    st.d        zero, t0, 0
    la.pcrel    t1, __bss_end__ - LONGSIZE

clear_bss_loop:
    addi.d      t0, t0, LONGSIZE
    st.d        zero, t0, 0
    bne         t0, t1, clear_bss_loop

#ifdef CONFIG_PAGE_SIZE_4KB
    /*
     * 配置 4KB 页大小
     *
     * CSR_STFILL 寄存器控制页表填充策略
     * 设置为 0 表示使用 4KB 页面
     */
    li.d        t0, 0
    li.d        t1, CSR_STFILL
    csrxchg     t0, t1, LOONGARCH_CSR_IMPCTL1
#endif

    /*
     * 清除 BIOS 定时器状态并禁用定时器
     *
     * 清除定时器中断标志，防止进入内核后立即触发中断
     */
    li.w    t0, CSR_TINTCLR_TI
    csrwr   t0, LOONGARCH_CSR_TINTCLR
    csrwr   $r0, LOONGARCH_CSR_TCFG

    /*
     * 关闭所有中断
     *
     * ECFG 寄存器控制中断使能
     * 写入 0 禁用所有中断
     */
    csrwr   $r0, LOONGARCH_CSR_ECFG

    /*
     * 使能外部扩展单元
     *
     * EUEN 寄存器控制外部扩展单元的使能：
     *   - FPEN: 浮点单元使能
     *   - LSXEN: LoongArch SIMD 扩展使能
     *   - LASXEN: LoongArch 高级 SIMD 扩展使能
     *   - LBTEN: LoongArch 二进制翻译使能
     */
    li.w    t0, CSR_EUEN_FPEN | CSR_EUEN_LSXEN | CSR_EUEN_LASXEN | CSR_EUEN_LBTEN
    csrwr   t0, LOONGARCH_CSR_EUEN

    /*
     * 初始化 per-CPU 基址
     *
     * KSave3 (PERCPU_BASE_KS) 和 u0 寄存器用于 per-CPU 数据基址
     * 初始化为 0，后续由内核设置
     */
    csrwr   zero, PERCPU_BASE_KS
    move    u0, zero

    /*
     * 设置系统调用栈
     *
     * 系统调用需要单独的栈，避免与内核栈混淆
     * 必须保证 16 字节对齐
     */
    la.pcrel    tp, syscall_stack
    PTR_LI      sp, (SYSCALL_STACK_SIZE - 0)
    PTR_ADD     sp, sp, tp

    /* 16 字节对齐：先右移 4 位再左移 4 位 */
    PTR_SRL     sp, sp, 4
    PTR_SLL     sp, sp, 4

    /* 保存系统调用栈指针 */
    set_saved_sp   sp, t0, t1

    /*
     * 设置初始化栈
     *
     * 内核初始化阶段使用的栈
     * 栈指针设置为 init_stack 末端减去 pt_regs 结构体大小
     * 保证 16 字节对齐
     */
    la.pcrel    tp, init_stack
    PTR_LI      sp, (STACK_SIZE - PT_SIZE)
    PTR_ADD     sp, sp, tp

    /* 16 字节对齐 */
    PTR_SRL     sp, sp, 4
    PTR_SLL     sp, sp, 4

    /*
     * 跳转到 C 内核入口点
     *
     * a0 = sp: 将栈指针作为第一个参数传递
     * start_kernel 函数不会返回
     */
    move    a0, sp
    bl      start_kernel

ENDPROC(_start)

#ifdef CONFIG_SMP
/**
 * @brief 从核启动入口
 *
 * @details LoongArch64 SMP 系统从核启动入口点
 *          执行流程：
 *          1. 配置直接映射窗口（DMW0-DMW3）
 *          2. 跳转到运行时虚拟地址
 *          3. 启用 MMU（分页机制）
 *          4. 清除 BIOS 定时器状态并禁用中断
 *          5. 使能外部扩展单元（FPU、LSX、LASX、LBT）
 *          6. 初始化 per-CPU 基址和栈
 *          7. 设置系统调用栈
 *          8. 设置初始化栈
 *          9. 跳转到 C 从核入口点
 *
 * @param a0 CPU ID（从 0 开始）
 *
 * @note 此函数由主核或固件调用，用于启动从核
 * @note 从核启动后进入独立执行流程
 *
 * @warning 此函数执行后不会返回
 */
ENTRY(_start_ap)

    /*
     * 配置直接映射窗口（DMW0-DMW3）
     * 与主核相同的配置
     */
    li.d    t0, CSR_DMW0_INIT
    csrwr   t0, LOONGARCH_CSR_DMWIN0
    li.d    t0, CSR_DMW1_INIT
    csrwr   t0, LOONGARCH_CSR_DMWIN1
    li.d    t0, CSR_DMW2_INIT
    csrwr   t0, LOONGARCH_CSR_DMWIN2
    li.d    t0, CSR_DMW3_INIT
    csrwr   t0, LOONGARCH_CSR_DMWIN3

    /*
     * 跳转到运行时虚拟地址
     */
    li.d    t0, CSR_DMW1_BASE
    pcaddi  t1, 0
    bstrins.d  t0, t1, (DMW_PABITS - 1), 0
    jirl    zero, t0, 0xc

#ifdef CONFIG_PAGE_SIZE_4KB
    /*
     * 配置 4KB 页大小
     */
    li.d        t0, 0
    li.d        t1, CSR_STFILL
    csrxchg     t0, t1, LOONGARCH_CSR_IMPCTL1
#endif

    /*
     * 启用 MMU（分页机制）
     */
    li.w        t0, 0xb0        /* PLV=0, IE=0, PG=1 */
    csrwr       t0, LOONGARCH_CSR_CRMD
    li.w        t0, 0x04        /* PLV=0, PIE=1, PWE=0 */
    csrwr       t0, LOONGARCH_CSR_PRMD
    li.w        t0, 0x00        /* FPE=0, SXE=0, ASXE=0, BTE=0 */
    csrwr       t0, LOONGARCH_CSR_EUEN

    /*
     * 清除 BIOS 定时器状态并禁用定时器
     */
    li.w    t0, CSR_TINTCLR_TI
    csrwr   t0, LOONGARCH_CSR_TINTCLR
    csrwr   $r0, LOONGARCH_CSR_TCFG

    /*
     * 关闭所有中断
     */
    csrwr   $r0, LOONGARCH_CSR_ECFG

    /*
     * 使能外部扩展单元
     */
    li.w    t0, CSR_EUEN_FPEN | CSR_EUEN_LSXEN | CSR_EUEN_LASXEN | CSR_EUEN_LBTEN
    csrwr   t0, LOONGARCH_CSR_EUEN

    /*
     * 保存系统调用异常栈
     *
     * 每个 CPU 都有独立的系统调用栈
     * 栈地址 = syscall_stack + CPU_ID * SYSCALL_STACK_SIZE
     */
    csrrd       t0, LOONGARCH_CSR_CPUID
    la.pcrel    t1, syscall_stack
    li.w        t2, SYSCALL_STACK_SIZE
    mul.w       t2, t2, t0
    PTR_ADD     t1, t1, t2

    PTR_LI      sp, (SYSCALL_STACK_SIZE - 0)
    PTR_ADD     sp, sp, t1

    /* 16 字节对齐 */
    PTR_SRL     sp, sp, 4
    PTR_SLL     sp, sp, 4

    /*
     * 初始化 per-CPU 基址
     *
     * 从 __per_cpu_offset 数组中获取当前 CPU 的偏移量
     * 存储在 u0 寄存器中，用于快速访问 per-CPU 数据
     */
    la.pcrel     t1, __per_cpu_offset
    li.w         t2, __SIZEOF_POINTER__
    mul.w        t2, t2, t0
    PTR_ADD      t1, t1, t2
    PTR_L        u0, t1, 0

    /* 保存系统调用栈指针 */
    set_saved_sp sp, t0, t1

    /*
     * 设置从核初始化栈
     *
     * 每个 CPU 都有独立的初始化栈
     * 栈地址 = init_stack + CPU_ID * STACK_SIZE
     */
    csrrd       t0, LOONGARCH_CSR_CPUID
    la.pcrel    t1, init_stack
    li.w        t2, STACK_SIZE
    mul.w       t2, t2, t0
    PTR_ADD     t1, t1, t2

    PTR_LI      sp, (STACK_SIZE - PT_SIZE)
    PTR_ADD     sp, sp, t1

    /* 16 字节对齐 */
    PTR_SRL     sp, sp, 4
    PTR_SLL     sp, sp, 4

    /*
     * 跳转到 C 从核入口点
     *
     * a0 = sp: 将栈指针作为第一个参数传递
     * start_ap 函数不会返回
     */
    move    a0, sp
    bl      start_ap

ENDPROC(_start_ap)
#endif /* CONFIG_SMP */

/**
 * @brief 初始化栈和系统调用栈
 *
 * @details 为每个 CPU 分配独立的初始化栈和系统调用栈
 *
 * @note init_stack 用于内核初始化
 * @note syscall_stack 用于系统调用处理
 * @note 每个 CPU 的栈大小为 STACK_SIZE 和 SYSCALL_STACK_SIZE
 */
.section ".data"
.align 8
init_stack:
    .space STACK_SIZE * CONFIG_MAX_CPUS
syscall_stack:
    .space SYSCALL_STACK_SIZE * CONFIG_MAX_CPUS
