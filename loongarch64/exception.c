/**
 * @file    exception.c
 * @brief   LoongArch64异常和中断处理实现
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.1
 *
 * @details 本文件实现了LoongArch64异常和中断处理相关功能
 *          - 异常向量表初始化
 *          - 中断处理（IRQ）
 *          - 系统调用处理
 *          - 各类异常处理（TLB、FPU、内存访问等）
 *          - 异常上下文管理
 *
 * @note MISRA-C:2012 合规
 * @warning 异常处理代码必须在关中断状态下执行
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

/*************************** 头文件包含 ****************************/
#include "ttosMM.h"

#include <barrier.h>
#include <context.h>
#include <cpu.h>
#include <exception.h>
#include <inttypes.h>
#include <math.h>
#include <process_signal.h>
#include <ptrace/ptrace.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <symtab.h>
#include <syscalls.h>
#include <tlb.h>
#include <trace/tracing.h>
#include <ttos.h>
#include <ttosProcess.h>
#include <ttos_pic.h>
#include <uaccess.h>

#include <ttosInterTask.inl>
#define KLOG_LEVEL KLOG_INFO
#undef KLOG_TAG
#define KLOG_TAG "Exception"
#include <klog.h>

/*************************** 外部变量声明 ****************************/
extern size_t intNestLevel[CONFIG_MAX_CPUS];
void ttosSchedule(void);
extern syscall_func syscall_table[CONFIG_SYSCALL_NUM];
extern syscall_func syscall_extent_table[CONFIG_EXTENT_SYSCALL_NUM];

/*************************** 全局变量 ****************************/
unsigned long eentry;    /* normal exception entry */
unsigned long tlbrentry; /* tlb exception entry */
long exception_handlers[VECSIZE * 128 / sizeof(long)] __attribute__((__aligned__(SZ_64K)));

/*************************** 函数实现 ****************************/
/**
 * @brief 异常向量表
 *
 * @details LoongArch64异常向量表，将异常码映射到对应的处理函数
 *          - TLB异常：TLBI、TLBL、TLBS、TLBM等
 *          - 内存异常：ADE（地址错误）、ALE（对齐错误）
 *          - 系统调用：SYS
 *          - 浮点和SIMD：FPU、LSX、LASX、FPE
 *          - 其他：断点、保留指令、观察点等
 */
void *exception_table[EXCCODE_INT_START] = {
    [0 ... EXCCODE_INT_START - 1] = handle_reserved,
    [EXCCODE_TLBI] = handle_tlb_load,      /**< TLB加载异常 */
    [EXCCODE_TLBL] = handle_tlb_load,      /**< TLB加载异常 */
    [EXCCODE_TLBS] = handle_tlb_store,     /**< TLB存储异常 */
    [EXCCODE_TLBM] = handle_tlb_modify,    /**< TLB修改异常 */
    [EXCCODE_TLBNR] = handle_tlb_protect0, /**< TLB保护异常0 */
    [EXCCODE_TLBNX] = handle_tlb_protect1, /**< TLB保护异常1 */
    [EXCCODE_TLBPE] = handle_tlb_protect2, /**< TLB保护异常2 */
    [EXCCODE_ADE] = handle_ade,            /**< 地址错误异常 */
    [EXCCODE_ALE] = handle_ale,            /**< 对齐错误异常 */
    [EXCCODE_BCE] = handle_bce,            /**< 边界检查异常 */
    [EXCCODE_SYS] = handle_sys,            /**< 系统调用 */
    [EXCCODE_BP] = handle_bp,              /**< 断点异常 */
    [EXCCODE_INE] = handle_ri,             /**< 保留指令异常 */
    [EXCCODE_IPE] = handle_ri,             /**< 保留指令异常 */
    [EXCCODE_FPDIS] = handle_fpu,          /**< FPU禁用异常 */
    [EXCCODE_LSXDIS] = handle_lsx,         /**< LSX禁用异常 */
    [EXCCODE_LASXDIS] = handle_lasx,       /**< LASX禁用异常 */
    [EXCCODE_FPE] = handle_fpe,            /**< 浮点异常 */
    [EXCCODE_WATCH] = handle_watch,        /**< 观察点异常 */
    [EXCCODE_BTDIS] = handle_lbt,          /**< LBT禁用异常 */
};
void local_flush_icache_range(unsigned long start, unsigned long end);
/**
 * @brief 安装CPU异常处理函数
 *
 * @details 将异常处理函数复制到异常向量表的指定位置
 *          并刷新指令缓存以确保新的处理函数生效
 *
 * @param offset 异常向量表中的偏移量
 * @param addr   异常处理函数地址
 * @param size   要复制的代码大小（字节）
 *
 * @return 无
 *
 * @note 此函数必须在关中断状态下调用
 * @note 复制后需要刷新指令缓存
 */
void set_handler(unsigned long offset, void *addr, unsigned long size)
{
    memcpy((void *)(eentry + offset), addr, size);
    local_flush_icache_range(eentry + offset, eentry + offset + size);
}
/**
 * @brief 设置中断向量间距
 *
 * @details 设置中断和异常入口之间的指令间距
 *          当前设置为128条指令（VECSIZE）
 *
 * @param size 向量间距大小（字节），必须是4的倍数
 *
 * @return 无
 *
 * @note 间距通过ECFG寄存器的VS字段配置
 * @note VS字段有效值为1-7，对应log2(size/4)
 */
static inline void setup_vint_size(unsigned int size)
{
    unsigned int vs;
    vs = (unsigned int)log2(size / 4U);
    if ((vs == 0U) || (vs > 7U))
    {
        printk("vint_size %d Not support yet", vs);
    }
    csr_xchg32(vs << CSR_ECFG_VS_SHIFT, CSR_ECFG_VS, LOONGARCH_CSR_ECFG);
}
/**
 * @brief 配置异常向量表基地址
 *
 * @details 设置异常入口和TLB异常入口的基地址到CSR寄存器
 *
 * @param 无
 *
 * @return 无
 *
 * @note eentry: 普通异常入口
 * @note tlbrentry: TLB异常入口（偏移80*VECSIZE）
 */
static void configure_exception_vector(void)
{
    eentry = (unsigned long)exception_handlers;
    tlbrentry = (unsigned long)exception_handlers + 80U * VECSIZE;
    csr_write64(eentry, LOONGARCH_CSR_EENTRY);
    csr_write64(eentry, LOONGARCH_CSR_MERRENTRY);
    csr_write64(tlbrentry, LOONGARCH_CSR_TLBRENTRY);
}
/**
 * @brief 异常处理初始化
 *
 * @details 初始化LoongArch64异常处理系统
 *          - 设置中断向量间距
 *          - 配置异常向量表基地址
 *          - 为前64个异常槽安装保留处理函数（仅CPU0）
 *          - 初始化TLB
 *
 * @param 无
 *
 * @return 无
 *
 * @note 仅在CPU0（启动CPU）上初始化异常槽
 */
void exception_init(void)
{
    int i = 0;
    uint32_t cpuid = 0U;
    setup_vint_size(VECSIZE);
    configure_exception_vector();
    cpuid = csr_read32(LOONGARCH_CSR_CPUID);
    if (cpuid == 0U)
    {
        for (i = 0; i < 64; i++)
        {
            set_handler((unsigned long)i * VECSIZE, handle_reserved, VECSIZE);
        }
    }
    tlb_init();
}
/**
 * @brief 陷阱初始化
 *
 * @details 初始化中断和异常向量处理函数
 *          - 为所有中断安装中断向量处理函数
 *          - 为所有异常安装对应的异常处理函数
 *
 * @param 无
 *
 * @return 无
 *
 * @note 必须在exception_init()之后调用
 */
void trap_init(void)
{
    long i;
    /* 设置中断向量处理函数 */
    for (i = EXCCODE_INT_START; i <= EXCCODE_INT_END; i++)
    {
        set_handler((unsigned long)i * VECSIZE, handle_vint, VECSIZE);
    }
    /* 设置异常向量处理函数 */
    for (i = EXCCODE_ADE; i <= EXCCODE_BTDIS; i++)
    {
        set_handler((unsigned long)i * VECSIZE, exception_table[i], VECSIZE);
    }
    local_flush_icache_range(eentry, eentry + 0x400U);
}
/**
 * @brief 异常模式下打印字符串
 *
 * @details 通过UART端口直接输出字符串（用于异常调试）
 *
 * @param s 要打印的字符串指针
 *
 * @return 无
 *
 * @note 此函数用于异常处理时的低级别调试输出
 * @note 直接访问UART硬件寄存器，不依赖任何系统服务
 */
void excPrintstr(char *s)
{
    unsigned long port = LOONGSON_UART0_symDebug;
    while (*s != '\0')
    {
        while (((*(volatile unsigned char *)(port + 5)) & 0x20U) == 0U)
        {
            /* 等待UART发送缓冲区空 */
        }
        *(volatile unsigned char *)port = (unsigned char)*s;
        s++;
    }
    while (((*(volatile unsigned char *)(port + 5)) & 0x20U) == 0U)
    {
        /* 等待最后一个字符发送完成 */
    }
}
/**
 * @brief 异常模式下打印十六进制数
 *
 * @details 通过UART端口以十六进制格式输出64位整数（用于异常调试）
 *
 * @param n 要打印的64位无符号整数
 *
 * @return 无
 *
 * @note 此函数用于异常处理时的低级别调试输出
 * @note 直接访问UART硬件寄存器，不依赖任何系统服务
 */
void excPrintnum(unsigned long long n)
{
    int i;
    int j;
    unsigned char a[40];
    unsigned long port = LOONGSON_UART0_symDebug;
    i = 0;
    /* 转换为十六进制（逆序） */
    do
    {
        a[i] = (unsigned char)(n % 16U);
        n = n / 16U;
        i++;
    } while (n > 0U);
    /* 输出十六进制数 */
    for (j = i - 1; j >= 0; j--)
    {
        if (a[j] >= 10U)
        {
            while (((*(volatile unsigned char *)(port + 5)) & 0x20U) == 0U)
            {
                /* 等待UART发送缓冲区空 */
            }
            *(volatile unsigned char *)port = (unsigned char)('a' + a[j] - 10U);
        }
        else
        {
            while (((*(volatile unsigned char *)(port + 5)) & 0x20U) == 0U)
            {
                /* 等待UART发送缓冲区空 */
            }
            *(volatile unsigned char *)port = (unsigned char)('0' + a[j]);
        }
    }
}
/**
 * @brief 检查是否为用户模式
 *
 * @details 根据特权级寄存器判断当前运行模式
 *
 * @param context 异常上下文指针
 *
 * @return true  用户模式（PLV_USER）
 * @return false 内核模式
 *
 * @note LoongArch有PLV0-PLV3四个特权级
 */
static bool is_user_mode(arch_int_context_t *context)
{
    return (((context)->csr_prmd & PLV_MASK) == PLV_USER);
}
/**
 * @brief 设置上下文类型
 *
 * @details 标记当前上下文的类型（系统调用、中断等）
 *          并保存原始参数值
 *
 * @param context 上下文指针（不能为NULL）
 * @param type    上下文类型（SYSCALL_CONTEXT、IRQ_CONTEXT等）
 *
 * @return 无
 *
 * @note orig_a0保存系统调用的第一个参数（用于返回处理）
 */
void set_context_type(struct arch_context *context, int type)
{
    context->type = type;
    context->orig_a0 = context->regs[4];
}
/**
 * @brief 保存异常上下文到PCB
 *
 * @details 将异常上下文复制到进程控制块的异常上下文区域
 *
 * @param pcb     进程控制块指针（不能为NULL）
 * @param context 异常上下文指针（不能为NULL）
 *
 * @return 无
 *
 * @note 用于信号处理和调试时恢复上下文
 */
static void save_exce_context(pcb_t pcb, struct arch_context *context)
{
    memcpy(&pcb->exception_context, context, sizeof(pcb->exception_context));
}
void do_work_pending(void *exp_frame);
void restore_hw_debug(pcb_t pcb);
/**
 * @brief 中断处理函数
 *
 * @details 处理硬件中断请求（IRQ）
 *          - 保存用户态异常上下文
 *          - 调用中断控制器处理中断
 *          - 触发任务调度
 *          - 处理挂起的信号和调试请求
 *
 * @param context 中断上下文指针（不能为NULL）
 *
 * @return 无
 *
 * @note 处理所有挂起的中断后返回
 * @note 支持多核中断处理
 */
void do_irq(arch_int_context_t *context)
{
    s32 ret;
    u32 from_cpu;
    u32 irq = 0U;
    s32 cpuid = 0;
    struct ttos_pic *pic_node;
    TASK_ID task = ttosGetRunningTask();
    pcb_t pcb = NULL;
    /* 如果是从用户态进入，保存异常上下文 */
    if (is_user_mode(context) && (task != NULL) && (task->ppcb != NULL))
    {
        pcb = (pcb_t)task->ppcb;
        save_exce_context(pcb, context);
        TTOS_TaskEnterKernelHook(task);
    }
    cpuid = cpuid_get();
    pic_node = ttos_pic_get_pic(PIC_FLAG_CORE);
    if (pic_node != NULL)
    {
        /* 处理所有挂起的中断 */
        while (pic_node->pic_ops->pic_ack(pic_node, &from_cpu, &irq) == 0)
        {
            TRACING_EVENT_ENTER(isr, irq, from_cpu);
            /* 增加中断嵌套层级 */
            ttosDisableScheduleLevel[cpuid]++;
            intNestLevel[cpuid]++;
            /* 调用中断处理函数 */
            ttos_pic_irq_handle(irq);
            /* 发送中断结束信号 */
            pic_node->pic_ops->pic_eoi(pic_node, irq, from_cpu);
            /* 减少中断嵌套层级 */
            intNestLevel[cpuid]--;
            ttosDisableScheduleLevel[cpuid]--;
            TRACING_EVENT_EXIT(isr, irq);
        }
    }
    /* 触发任务调度 */
    ttosSchedule();
    set_context_type(context, IRQ_CONTEXT);
    /* 返回用户态前的处理 */
    if (is_user_mode(context))
    {
        pcb_t pcb_local = ttosProcessSelf();
        do_work_pending(context);
        if (pcb_local->group_leader->ptrace)
        {
            restore_hw_debug(pcb_local->group_leader);
        }
        TTOS_TaskEnterUserHook(pcb_local->taskControlId);
    }
}
/**
 * @brief 系统调用处理函数
 *
 * @details 处理用户态发起的系统调用请求
 *          - 更新ERA指针到下一条指令
 *          - 保存异常上下文
 *          - 分发到对应的系统调用处理函数
 *          - 返回前处理挂起的信号和调试请求
 *
 * @param context 系统调用上下文指针（不能为NULL）
 *
 * @return 无
 *
 * @note LoongArch系统调用约定：
 *       - a0-a6(r4-r10): 参数
 *       - a7(r11): 系统调用号
 *       - r4: 返回值
 *
 * @note 返回时需要跳过系统调用指令（+4字节）
 */
void do_syscall(arch_exception_context_t *context)
{
    register unsigned long sp __asm__("$sp");
    int64_t ret = 0;
    int32_t syscall_num = 0;
    TASK_ID task = ttosGetRunningTask();
    pcb_t pcb = NULL;
    /*
     * csr_era保存触发系统调用的指令的PC
     * LoongArch每条指令大小都是32位（4字节）
     * 系统调用返回时需要返回到下一条指令，所以需要对csr_era+4
     */
    context->csr_era += 4L;
    set_context_type(context, SYSCALL_CONTEXT);
    /* 如果是从用户态进入，保存异常上下文 */
    if (is_user_mode(context) && (task != NULL) && (task->ppcb != NULL))
    {
        pcb = (pcb_t)task->ppcb;
        save_exce_context(pcb, context);
        TTOS_TaskEnterKernelHook(task);
    }
    /* 系统约定a0~a6(r4~r10)用于传递参数，a7(r11)用于传递系统调用号 */
    syscall_num = (int32_t)context->regs[11];
    /* 处理扩展系统调用 */
    if (is_extent_syscall_num(syscall_num))
    {
        syscall_num -= CONFIG_EXTENT_SYSCALL_NUM_START;
        if (syscall_num == 0)
        {
            ret = syscall_extent_table[syscall_num]((long)context, 0L, 0L, 0L, 0L, 0L);
        }
        else
        {
            ret = syscall_extent_table[syscall_num](context->orig_a0, context->regs[5], context->regs[6],
                                                    context->regs[7], context->regs[8], context->regs[9]);
        }
        context->regs[4] = (uint64_t)ret;
        return;
    }
    /* 处理标准系统调用 */
    if (syscall_num >= CONFIG_SYSCALL_NUM)
    {
        KLOG_I("syscall num %d great than:%d\n", syscall_num, CONFIG_SYSCALL_NUM - 1);
    }
    else
    {
        if (syscall_num == __NR_rt_sigreturn)
        {
            /* 信号处理函数调用完毕的返回系统调用 */
            rt_sigreturn(context);
        }
        else if (syscall_table[syscall_num] != NULL)
        {
            TRACING_EVENT_ENTER(syscall, syscall_num, syscall_getname(syscall_num), context->orig_a0, context->regs[5],
                                context->regs[6], context->regs[7], context->regs[8], context->regs[9]);
            ret = syscall_table[syscall_num](context->orig_a0, context->regs[5], context->regs[6], context->regs[7],
                                             context->regs[8], context->regs[9]);
            context->regs[4] = (uint64_t)ret;
            TRACING_EVENT_EXIT(syscall, syscall_getname(syscall_num), context->regs[4]);
        }
        else
        {
            KLOG_E("syscall_table[%d] is NULL\n", syscall_num);
        }
    }
    /* 返回用户态前的处理 */
    if (is_user_mode(context))
    {
        pcb_t pcb_local = ttosProcessSelf();
        do_work_pending(context);
        if (pcb_local->group_leader->ptrace)
        {
            restore_hw_debug(pcb_local->group_leader);
        }
        TTOS_TaskEnterUserHook(pcb_local->taskControlId);
    }
}
/**
 * @brief 异常处理程序（核心函数）
 *
 * @details 处理所有类型的内核异常
 *          打印异常信息和寄存器状态
 *          进入死循环等待系统复位
 *
 * @param context 异常上下文指针（不能为NULL）
 *
 * @return 无（此函数不会返回）
 *
 * @note 此函数打印详细的异常信息后进入死循环
 * @note 仅用于致命异常，系统无法恢复
 */
void do_exception(arch_exception_context_t *context)
{
    long Ecode = (long)((context->csr_estat & ((0x3fL) << 16)) >> 16);
    long EsubCode = (long)((context->csr_estat & ((0x1ffL) << 22)) >> 22);
    long badins = (long)csr_read64(LOONGARCH_CSR_BADI);
    KLOG_EMERG("================Kernel Exception================");
    KLOG_EMERG("CPUID: %u, TaskID: %p", csr_read32(LOONGARCH_CSR_CPUID), ttosGetRunningTask());
    KLOG_EMERG("RegMap:");
    KLOG_EMERG("EPC:       %p", (void *)context->csr_era);
    KLOG_EMERG("ESTAT:     %p", (void *)context->csr_estat);
    KLOG_EMERG("ECode:     %lx", Ecode);
    KLOG_EMERG("EsubCode:  %lx", EsubCode);
    KLOG_EMERG("BadVA:     %p", (void *)context->csr_badvaddr);
    KLOG_EMERG("PRMD:      %p", (void *)context->csr_prmd);
    KLOG_EMERG("ECFG:      %p", (void *)context->csr_ecfg);
    KLOG_EMERG("CRMD:      %p", (void *)context->csr_crmd);
    KLOG_EMERG("BadI:      %lx", badins);
    KLOG_EMERG("task name: %s", ttosGetRunningTaskName());
    show_all_register(context);
    /* 进入死循环，等待看门狗复位或人工干预 */
    for (;;)
    {
        /* 系统已无法恢复，等待复位 */
    }
}
/**
 * @brief 地址错误异常处理
 *
 * @details 处理地址错误异常（ADE）
 *
 * @param context 异常上下文指针（不能为NULL）
 *
 * @return 无（此函数不会返回）
 *
 * @note ADE通常由访问无效内存地址引起
 */
void do_ade(arch_exception_context_t *context)
{
    KLOG_EMERG("Ade exception");
    do_exception(context);
}
/**
 * @brief 对齐错误异常处理
 *
 * @details 处理地址对齐错误异常（ALE）
 *
 * @param context 异常上下文指针（不能为NULL）
 *
 * @return 无（此函数不会返回）
 *
 * @note ALE通常由未对齐的内存访问引起
 */
void do_ale(arch_exception_context_t *context)
{
    KLOG_EMERG("Ale exception");
    do_exception(context);
}
/**
 * @brief 边界检查异常处理
 *
 * @details 处理边界检查异常（BCE）
 *
 * @param context 异常上下文指针（不能为NULL）
 *
 * @return 无（此函数不会返回）
 */
void do_bce(arch_exception_context_t *context)
{
    KLOG_EMERG("Bce exception");
    do_exception(context);
}
/**
 * @brief 断点异常处理
 *
 * @details 处理断点异常（BP）
 *
 * @param context 异常上下文指针（不能为NULL）
 *
 * @return 无（此函数不会返回）
 *
 * @note BP由软件断点指令触发
 */
void do_bp(arch_exception_context_t *context)
{
    KLOG_EMERG("BP exception");
    do_exception(context);
}
/**
 * @brief 浮点单元禁用异常处理
 *
 * @details 处理FPU禁用异常（FPDIS）
 *
 * @param context 异常上下文指针（不能为NULL）
 *
 * @return 无（此函数不会返回）
 *
 * @note 通常需要在此处启用FPU并恢复执行
 */
void do_fpu(arch_exception_context_t *context)
{
    KLOG_EMERG("Fpu exception");
    do_exception(context);
}
/**
 * @brief 浮点异常处理
 *
 * @details 处理浮点运算异常（FPE）
 *
 * @param context 异常上下文指针（不能为NULL）
 *
 * @return 无（此函数不会返回）
 *
 * @note FPE由浮点运算错误（除零、溢出等）引起
 */
void do_fpe(arch_exception_context_t *context)
{
    KLOG_EMERG("Fpe exception");
    do_exception(context);
}
/**
 * @brief LSX禁用异常处理
 *
 * @details 处理LoongArch SIMD扩展禁用异常
 *
 * @param context 异常上下文指针（不能为NULL）
 *
 * @return 无（此函数不会返回）
 */
void do_lsx(arch_exception_context_t *context)
{
    KLOG_EMERG("Lsx exception");
    do_exception(context);
}
/**
 * @brief LASX禁用异常处理
 *
 * @details 处理LoongArch高级SIMD扩展禁用异常
 *
 * @param context 异常上下文指针（不能为NULL）
 *
 * @return 无（此函数不会返回）
 */
void do_lasx(arch_exception_context_t *context)
{
    KLOG_EMERG("Lasx exception");
    do_exception(context);
}
/**
 * @brief LBT禁用异常处理
 *
 * @details 处理LoongArch二进制翻译禁用异常
 *
 * @param context 异常上下文指针（不能为NULL）
 *
 * @return 无（此函数不会返回）
 */
void do_lbt(arch_exception_context_t *context)
{
    KLOG_EMERG("Lbt exception");
    do_exception(context);
}
/**
 * @brief 保留指令异常处理
 *
 * @details 处理保留指令异常（RI）
 *
 * @param context 异常上下文指针（不能为NULL）
 *
 * @return 无（此函数不会返回）
 *
 * @note RI由执行未实现或保留的指令引起
 */
void do_ri(arch_exception_context_t *context)
{
    KLOG_EMERG("Ri exception");
    do_exception(context);
}
/**
 * @brief 观察点异常处理
 *
 * @details 处理观察点异常（WATCH）
 *
 * @param context 异常上下文指针（不能为NULL）
 *
 * @return 无（此函数不会返回）
 *
 * @note WATCH由硬件观察点匹配触发
 */
void do_watch(arch_exception_context_t *context)
{
    KLOG_EMERG("Watch exception");
    do_exception(context);
}
/**
 * @brief TLB异常测试函数
 *
 * @details 测试TLB异常处理功能
 *          打印TLB异常的地址信息
 *
 * @param 无
 *
 * @return 无（此函数不会返回）
 *
 * @note 此函数用于调试TLB异常处理
 * @note 调用后会进入死循环
 */
void test_tlb_exception(void)
{
    unsigned long tlbEPC = 0UL;
    unsigned long badv = 0UL;
    tlbEPC = csr_read64(LOONGARCH_CSR_TLBRERA);
    badv = csr_read64(LOONGARCH_CSR_TLBRBADV);
    excPrintstr("****TLBRE EXCEPTION****\n\r");
    excPrintstr("TLBEPC: 0x");
    excPrintnum(tlbEPC);
    excPrintstr("\n\r");
    excPrintstr("TLBRBADV: 0x");
    excPrintnum(badv);
    excPrintstr("\n\r");
    excPrintstr("--------------------------\n\r");
    /* 进入死循环 */
    for (;;)
    {
        /* 等待调试 */
    }
}