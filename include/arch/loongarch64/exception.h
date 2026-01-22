/**
 * @file    exception.h
 * @brief   LoongArch64异常和中断处理接口
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 本文件定义了LoongArch64异常和中断处理相关的接口
 *          - 异常码定义
 *          - 中断号定义
 *          - 异常处理函数声明
 *          - 异常栈管理
 *
 * @note MISRA-C:2012 合规
 * @warning 异常处理函数必须在关中断状态下调用
 *
 * @copyright Copyright (c) 2025 AISafe64 Team
 */

#ifndef _EXCEPTION_H
#define _EXCEPTION_H

/************************头 文 件******************************/
#include <percpu.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/************************宏 定 义******************************/

/**
 * @brief IRQ栈大小
 */
#define IRQ_STACK_SIZE THREAD_SIZE

/**
 * @brief IRQ栈起始位置偏移
 */
#define IRQ_STACK_START (IRQ_STACK_SIZE - 16U)

    /************************外部变量******************************/
    DECLARE_PER_CPU(unsigned long, irq_stack);

    /************************类型定义******************************/

    /************************宏 � 数******************************/

    /**
     * @brief 判断栈指针是否在IRQ栈上
     *
     * @details 检查给定的栈指针是否在指定CPU的IRQ栈范围内
     *
     * @param cpu CPU编号
     * @param sp  栈指针
     *
     * @return true  在IRQ栈上
     * @return false 不在IRQ栈上
     *
     * @note IRQ栈结构：
     *       top ------------
     *           | task sp  | <- irq_stack[cpu] + IRQ_STACK_START
     *           ------------
     *           |          | <- First frame of IRQ context
     *           ------------
     *
     *       task sp保存任务栈指针，指向异常入口的pt_regs结构
     */
    static inline bool on_irq_stack(int cpu, unsigned long sp)
    {
        unsigned long low = per_cpu(irq_stack, cpu);
        unsigned long high = low + IRQ_STACK_SIZE;

        return ((low <= sp) && (sp <= high));
    }

    /************************异常向量表******************************/
    extern void *exception_table[];

    /************************异常码定义******************************/

    /**
     * @defgroup ExceptionCodes 异常码定义
     * @brief LoongArch64异常码（ExStatus.ExcCode）
     * @{
     */

#define EXCCODE_RSV 0U      /**< 保留 */
#define EXCCODE_TLBL 1U     /**< TLB加载缺失 */
#define EXCCODE_TLBS 2U     /**< TLB存储缺失 */
#define EXCCODE_TLBI 3U     /**< TLB取指缺失 */
#define EXCCODE_TLBM 4U     /**< TLB修改错误 */
#define EXCCODE_TLBNR 5U    /**< TLB读禁止异常 */
#define EXCCODE_TLBNX 6U    /**< TLB执行禁止异常 */
#define EXCCODE_TLBPE 7U    /**< TLB特权错误 */
#define EXCCODE_ADE 8U      /**< 地址错误 */
#define EXSUBCODE_ADEF 0U   /**< 取指时的地址错误 */
#define EXSUBCODE_ADEM 1U   /**< 访问内存时的地址错误 */
#define EXCCODE_ALE 9U      /**< 未对齐访问 */
#define EXCCODE_BCE 10U     /**< 边界检查错误 */
#define EXCCODE_SYS 11U     /**< 系统调用 */
#define EXCCODE_BP 12U      /**< 断点 */
#define EXCCODE_INE 13U     /**< 指令不存在 */
#define EXCCODE_IPE 14U     /**< 指令特权错误 */
#define EXCCODE_FPDIS 15U   /**< FPU禁用 */
#define EXCCODE_LSXDIS 16U  /**< LSX（SIMD扩展）禁用 */
#define EXCCODE_LASXDIS 17U /**< LASX（高级SIMD扩展）禁用 */
#define EXCCODE_FPE 18U     /**< 浮点异常 */
#define EXCSUBCODE_FPE 0U   /**< 浮点异常 */
#define EXCSUBCODE_VFPE 1U  /**< 向量浮点异常 */
#define EXCCODE_WATCH 19U   /**< 观察点异常 */
#define EXCSUBCODE_WPEF 0U  /**< 取指时的观察点异常 */
#define EXCSUBCODE_WPEM 1U  /**< 访问内存时的观察点异常 */
#define EXCCODE_BTDIS 20U   /**< 二进制翻译禁用 */
#define EXCCODE_BTE 21U     /**< 二进制翻译异常 */
#define EXCCODE_GSPR 22U    /**< 客机特权错误 */
#define EXCCODE_HVC 23U     /**< 超级调用 */
#define EXCCODE_GCM 24U     /**< 客机CSR修改 */
#define EXCSUBCODE_GCSC 0U  /**< 软件引起 */
#define EXCSUBCODE_GCHC 1U  /**< 硬件引起 */
#define EXCCODE_SE 25U      /**< 安全异常 */

    /** @} */ /* end of ExceptionCodes */

    /************************中断号定义******************************/

    /**
     * @defgroup InterruptNumbers 中断号定义
     * @brief LoongArch64中断号
     * @{
     */

#define INT_SWI0 0U  /**< 软件中断0 */
#define INT_SWI1 1U  /**< 软件中断1 */
#define INT_HWI0 2U  /**< 硬件中断0 */
#define INT_HWI1 3U  /**< 硬件中断1 */
#define INT_HWI2 4U  /**< 硬件中断2 */
#define INT_HWI3 5U  /**< 硬件中断3 */
#define INT_HWI4 6U  /**< 硬件中断4 */
#define INT_HWI5 7U  /**< 硬件中断5 */
#define INT_HWI6 8U  /**< 硬件中断6 */
#define INT_HWI7 9U  /**< 硬件中断7 */
#define INT_PCOV 10U /**< 性能计数器溢出 */
#define INT_TI 11U   /**< 定时器 */
#define INT_IPI 12U  /**< 核间中断 */
#define INT_NMI 13U  /**< 不可屏蔽中断 */
#define INT_AVEC 14U /**< 向量中断 */

/** @} */ /* end of InterruptNumbers */

/**
 * @brief 中断对应的异常码范围
 */
#define EXCCODE_INT_NUM (INT_AVEC + 1U)
#define EXCCODE_INT_START 64U
#define EXCCODE_INT_END (EXCCODE_INT_START + EXCCODE_INT_NUM - 1U)

/************************大小常量定义******************************/
#define SZ_1K 0x00000400UL   /**< 1KB */
#define SZ_2K 0x00000800UL   /**< 2KB */
#define SZ_4K 0x00001000UL   /**< 4KB */
#define SZ_8K 0x00002000UL   /**< 8KB */
#define SZ_16K 0x00004000UL  /**< 16KB */
#define SZ_32K 0x00008000UL  /**< 32KB */
#define SZ_64K 0x00010000UL  /**< 64KB */
#define SZ_128K 0x00020000UL /**< 128KB */
#define SZ_256K 0x00040000UL /**< 256KB */
#define SZ_512K 0x00080000UL /**< 512KB */

/**
 * @brief UART调试端口地址
 */
#define LOONGSON_UART0_symDebug 0x800000001fe001e0ULL

/************************函数声明******************************/
#ifndef ASM_USE

    /**
     * @brief 保留异常处理函数
     */
    void handle_reserved(void);

    /**
     * @brief TLB加载异常处理
     */
    void handle_tlb_load(void);

    /**
     * @brief TLB存储异常处理
     */
    void handle_tlb_store(void);

    /**
     * @brief TLB修改异常处理
     */
    void handle_tlb_modify(void);

    /**
     * @brief TLB保护异常0处理
     */
    void handle_tlb_protect0(void);

    /**
     * @brief TLB保护异常1处理
     */
    void handle_tlb_protect1(void);

    /**
     * @brief TLB保护异常2处理
     */
    void handle_tlb_protect2(void);

    /**
     * @brief 地址错误异常处理
     */
    void handle_ade(void);

    /**
     * @brief 对齐错误异常处理
     */
    void handle_ale(void);

    /**
     * @brief 边界检查错误异常处理
     */
    void handle_bce(void);

    /**
     * @brief 系统调用处理
     */
    void handle_sys(void);

    /**
     * @brief 断点异常处理
     */
    void handle_bp(void);

    /**
     * @brief 保留指令异常处理
     */
    void handle_ri(void);

    /**
     * @brief FPU禁用异常处理
     */
    void handle_fpu(void);

    /**
     * @brief LSX禁用异常处理
     */
    void handle_lsx(void);

    /**
     * @brief LASX禁用异常处理
     */
    void handle_lasx(void);

    /**
     * @brief 浮点异常处理
     */
    void handle_fpe(void);

    /**
     * @brief 观察点异常处理
     */
    void handle_watch(void);

    /**
     * @brief LBT禁用异常处理
     */
    void handle_lbt(void);

    /**
     * @brief 向量中断处理
     */
    void handle_vint(void);

    /**
     * @brief 异常处理初始化
     *
     * @details 初始化LoongArch64异常处理系统
     *          - 设置异常向量间距
     *          - 配置异常向量表
     *          - 初始化TLB
     */
    void exception_init(void);

#endif /* !ASM_USE */

/************************内联函数******************************/

/************************C++兼容性******************************/
#ifdef __cplusplus
}
#endif

#endif /* _EXCEPTION_H */