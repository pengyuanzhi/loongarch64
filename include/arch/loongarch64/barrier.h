/**
 * @file    barrier.h
 * @brief   LoongArch64内存屏障接口
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 本文件定义了LoongArch64内存屏障和同步接口
 *          - 数据缓存屏障（DBAR）
 *          - 指令屏障（IBAR）
 *          - 编译器屏障
 *          - 原子操作内存屏障
 *          - Load-Acquire/Store-Release语义
 *
 * @note MISRA-C:2012 合规
 * @warning 多核环境下必须正确使用内存屏障
 *
 * @par 屏障提示编码（Hint Encoding）
 * - Bit4: 排序或完成（0=完成，1=排序）
 * - Bit3: 前读屏障（0=真，1=假）
 * - Bit2: 前写屏障（0=真，1=假）
 * - Bit1: 后读屏障（0=真，1=假）
 * - Bit0: 后写屏障（0=真，1=假）
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef __ASM_BARRIER_H
#define __ASM_BARRIER_H

/************************头 文 件******************************/
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/************************宏 定 义******************************/

/**
 * @defgroup BarrierHints 屏障提示编码
 * @brief LoongArch64 DBAR指令提示编码
 * @{
 */

/**
 * @brief 数据缓存屏障指令
 *
 * @details 插入数据缓存同步屏障
 *
 * @param hint 屏障提示编码
 */
#define DBAR(hint) __asm__ __volatile__("dbar %0 " : : "I"(hint) : "memory")

/**
 * @brief 指令屏障指令
 *
 * @details 刷新指令流水线，确保指令一致性
 */
#define IBAR() __asm__ __volatile__("\tibar 0\n" ::: "memory")

/**
 * @brief 完成屏障（Completion Barrier）
 * @{
 */
#define crwrw 0b00000 /**< 读写完成屏障 */
#define cr_r_ 0b00101 /**< 读完成屏障 */
#define c_w_w 0b01010 /**< 写完成屏障 */
/** @} */

/**
 * @brief 排序屏障（Ordering Barrier）
 * @{
 */
#define orwrw 0b10000 /**< 读写排序屏障 */
#define or_r_ 0b10101 /**< 读排序屏障 */
#define o_w_w 0b11010 /**< 写排序屏障 */
#define orw_w 0b10010 /**< 写-读排序屏障 */
#define or_rw 0b10100 /**< 读-写排序屏障 */
/** @} */

/** @} */

/************************完成屏障宏******************************/

/**
 * @defgroup CompletionBarriers 完成屏障宏
 * @{
 */

/**
 * @brief 完成同步屏障（读写）
 */
#define c_sync() DBAR(crwrw)

/**
 * @brief 读完成同步屏障
 */
#define c_rsync() DBAR(cr_r_)

/**
 * @brief 写完成同步屏障
 */
#define c_wsync() DBAR(c_w_w)

/** @} */

/************************排序屏障宏******************************/

/**
 * @defgroup OrderingBarriers 排序屏障宏
 * @{
 */

/**
 * @brief 排序同步屏障（读写）
 */
#define o_sync() DBAR(orwrw)

/**
 * @brief 读排序同步屏障
 */
#define o_rsync() DBAR(or_r_)

/**
 * @brief 写排序同步屏障
 */
#define o_wsync() DBAR(o_w_w)

/**
 * @brief 读-写排序屏障（用于Load-Acquire）
 */
#define ldacq_mb() DBAR(or_rw)

/**
 * @brief 写-读排序屏障（用于Store-Release）
 */
#define strel_mb() DBAR(orw_w)

/** @} */

/************************通用屏障宏******************************/

/**
 * @defgroup GeneralBarriers 通用屏障宏
 * @{
 */

/**
 * @brief 通用内存屏障
 *
 * @details 确保所有内存访问完成
 */
#define mb() c_sync()

/**
 * @brief 读内存屏障
 *
 * @details 确保所有读操作完成
 */
#define rmb() c_rsync()

/**
 * @brief 写内存屏障
 *
 * @details 确保所有写操作完成
 */
#define wmb() c_wsync()

/**
 * @brief I/O内存屏障
 *
 * @details 确保I/O操作完成
 */
#define iob() c_sync()

/**
 * @brief 写回刷新
 *
 * @details 刷新写回缓冲区
 */
#define wbflush() c_sync()

/** @} */

/************************SMP屏障宏******************************/

/**
 * @defgroup SMPBarriers SMP屏障宏
 * @{
 */

/**
 * @brief SMP内存屏障（使用排序屏障）
 */
#define __smp_mb() o_sync()

/**
 * @brief SMP读内存屏障
 */
#define __smp_rmb() o_rsync()

/**
 * @brief SMP写内存屏障
 */
#define __smp_wmb() o_wsync()

/**
 * @brief 内存屏障（单核/SMP通用）
 */
#define smp_mb() mb()

/**
 * @brief SMP写屏障
 */
#define smp_wmb() __smp_wmb()

/** @} */

/**
 * @brief 指令同步屏障
 *
 * @details 确保指令流水线同步
 */
#define isb() __asm__ __volatile__("\tibar 0\n" ::: "memory")

/************************LLSC屏障宏******************************/

/**
 * @brief LLSC（Load-Linked/Store-Conditional）弱内存屏障
 *
 * @details 在SMP配置下使用读屏障，否则为空
 */
#ifdef CONFIG_SMP
#define __WEAK_LLSC_MB "   dbar 0x700  \n"
#else
#define __WEAK_LLSC_MB "           \n"
#endif

/************************原子操作屏障******************************/

/**
 * @brief 原子操作前的内存屏障
 */
#define __smp_mb__before_atomic() barrier()

/**
 * @brief 原子操作后的内存屏障
 */
#define __smp_mb__after_atomic() barrier()

/************************防止推测执行******************************/

/**
 * @brief 防止数组索引推测执行
 *
 * @details 当index < size时生成全1掩码，否则生成0掩码
 *          用于防止旁路攻击（Spectre等）
 *
 * @param index 数组索引
 * @param size  数组大小
 *
 * @return ~0（当index < size）或 0（当index >= size）
 */
#define array_index_mask_nospec array_index_mask_nospec
static inline unsigned long array_index_mask_nospec(unsigned long index, unsigned long size)
{
    unsigned long mask;

    __asm__ __volatile__("sltu   %0, %1, %2\n\t"
#if (__SIZEOF_LONG__ == 4)
                         "sub.w  %0, $zero, %0\n\t"
#elif (__SIZEOF_LONG__ == 8)
                         "sub.d  %0, $zero, %0\n\t"
#endif
                         : "=r"(mask)
                         : "r"(index), "r"(size)
                         :);

    return mask;
}

/************************Load-Acquire/Store-Release****************/

/**
 * @defgroup AcquireRelease Load-Acquire/Store-Release语义
 * @{
 */

/**
 * @brief Load-Acquire语义
 *
 * @details 带内存屏障的加载操作，确保后续操作不会被重排到加载之前
 */
#define __smp_load_acquire(p) \
    ({ \
        typeof(*p) ___p1 = READ_ONCE(*p); \
        compiletime_assert_atomic_type(*p); \
        ldacq_mb(); \
        ___p1; \
    })

/**
 * @brief Store-Release语义
 *
 * @details 带内存屏障的存储操作，确保前面的操作不会被重排到存储之后
 */
#define __smp_store_release(p, v) \
    do \
    { \
        compiletime_assert_atomic_type(*p); \
        strel_mb(); \
        WRITE_ONCE(*p, v); \
    } while (0)

/**
 * @brief 带内存屏障的存储操作
 *
 * @details 使用原子交换指令实现带屏障的存储
 */
#define __smp_store_mb(p, v) \
    do \
    { \
        union \
        { \
            typeof(p) __val; \
            char __c[1]; \
        } __u = {.__val = (__force typeof(p))(v)}; \
        unsigned long __tmp; \
        switch (sizeof(p)) \
        { \
        case 1: \
            *(volatile __u8 *)&p = *(__u8 *)__u.__c; \
            __smp_mb(); \
            break; \
        case 2: \
            *(volatile __u16 *)&p = *(__u16 *)__u.__c; \
            __smp_mb(); \
            break; \
        case 4: \
            __asm__ __volatile__("amswap_db.w %[tmp], %[val], %[mem]  \n" \
                                 : [mem] "+ZB"(*(u32 *)&p), [tmp] "=&r"(__tmp) \
                                 : [val] "r"(*(__u32 *)__u.__c) \
                                 :); \
            break; \
        case 8: \
            __asm__ __volatile__("amswap_db.d %[tmp], %[val], %[mem]  \n" \
                                 : [mem] "+ZB"(*(u64 *)&p), [tmp] "=&r"(__tmp) \
                                 : [val] "r"(*(__u64 *)__u.__c) \
                                 :); \
            break; \
        } \
    } while (0)

/** @} */

/************************C++兼容性******************************/
#ifdef __cplusplus
}
#endif

#endif /* __ASM_BARRIER_H */
