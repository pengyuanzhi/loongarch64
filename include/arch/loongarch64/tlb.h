/**
 * @file    tlb.h
 * @brief   LoongArch64 TLB操作接口
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件定义LoongArch64 TLB（Translation Lookaside Buffer）操作接口
 *          - TLB刷新指令
 *          - TLB读写操作
 *          - TLB无效化操作
 *          - TLB异常处理函数
 *
 * @note MISRA-C:2012 合规
 * @note TLB用于加速虚拟地址到物理地址的转换
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef _TLB_H
#define _TLB_H

/************************头 文 件******************************/
#include <atomic.h>
#include <ttos_arch.h>
#include <system/types.h>

/************************宏 定 义******************************/

/**
 * @defgroup TLBVectorSize TLB向量大小
 * @{
 */

/**
 * @brief TLB向量大小
 */
#define VECSIZE    0x200U

/** @} */

/************************类型定义******************************/

/**
 * @defgroup TLBInvalidateOps TLB无效化操作类型
 * @{
 */

/**
 * @brief TLB无效化操作枚举
 *
 * @details 定义各种TLB无效化操作的类型码
 */
enum invtlb_ops
{
    /* 主TLB无效化操作 */
    INVTLB_ALL = 0x0,                      /**< 无效化所有TLB */
    INVTLB_CURRENT_ALL = 0x1,               /**< 无效化当前所有TLB */
    INVTLB_CURRENT_GTRUE = 0x2,             /**< 无效化当前所有G=1的TLB项 */
    INVTLB_CURRENT_GFALSE = 0x3,            /**< 无效化当前所有G=0的TLB项 */
    INVTLB_GFALSE_AND_ASID = 0x4,           /**< 无效化当前G=0且ASID匹配的TLB项 */
    INVTLB_ADDR_GFALSE_AND_ASID = 0x5,      /**< 无效化当前地址且G=0且ASID匹配的TLB项 */
    INVTLB_ADDR_GTRUE_OR_ASID = 0x6,        /**< 无效化当前地址且G=1或ASID匹配的TLB项 */

    /* 客户TLB无效化操作 */
    INVGTLB_GID = 0x9,                      /**< 无效化客户TLB中GID匹配的项 */
    INVGTLB_GID_GTRUE = 0xa,                /**< 无效化客户TLB中G=1且GID匹配的项 */
    INVGTLB_GID_GFALSE = 0xb,               /**< 无效化客户TLB中G=0且GID匹配的项 */
    INVGTLB_GID_GFALSE_ASID = 0xc,          /**< 无效化客户TLB中G=0且GID和ASID匹配的项 */
    INVGTLB_GID_GFALSE_ASID_ADDR = 0xd,     /**< 无效化客户TLB中G=0且GID、ASID和地址匹配的项 */
    INVGTLB_GID_GTRUE_ASID_ADDR = 0xe,      /**< 无效化客户TLB中G=1且GID、ASID和地址匹配的项 */
    INVGTLB_ALLGID_GVA_TO_GPA = 0x10,       /**< 无效化所有GID的GVA→GPA客户TLB */
    INVTLB_ALLGID_GPA_TO_HPA = 0x11,        /**< 无效化所有GID的GPA→HPA TLB */
    INVTLB_ALLGID = 0x12,                   /**< 无效化所有GID的TLB（包括GVA→GPA和GPA→HPA） */
    INVGTLB_GID_GVA_TO_GPA = 0x13,          /**< 无效化GID匹配的GVA→GPA客户TLB */
    INVTLB_GID_GPA_TO_HPA = 0x14,           /**< 无效化GID匹配的GPA→HPA TLB */
    INVTLB_GID_ALL = 0x15,                  /**< 无效化GID匹配的所有TLB（包括GVA→GPA和GPA→HPA） */
    INVTLB_GID_ADDR = 0x16                  /**< 无效化GID匹配且地址匹配的GPA→HPA TLB */
};

/** @} */

/************************外部变量******************************/
/************************外部函数******************************/

/**
 * @defgroup TLBHandlers TLB异常处理函数
 * @{
 */

/**
 * @brief TLB加载异常处理
 */
extern void handle_tlb_load(void);

/**
 * @brief TLB存储异常处理
 */
extern void handle_tlb_store(void);

/**
 * @brief TLB修改异常处理
 */
extern void handle_tlb_modify(void);

/**
 * @brief TLB重填异常处理
 */
extern void handle_tlb_refill(void);

/**
 * @brief TLB保护异常处理
 */
extern void handle_tlb_protect(void);

/**
 * @brief TLB加载页表遍历异常处理
 */
extern void handle_tlb_load_ptw(void);

/**
 * @brief TLB存储页表遍历异常处理
 */
extern void handle_tlb_store_ptw(void);

/**
 * @brief TLB修改页表遍历异常处理
 */
extern void handle_tlb_modify_ptw(void);

/**
 * @brief TLB初始化
 */
extern void tlb_init(void);

/**
 * @brief 显示所有寄存器
 *
 * @param ptRegs 架构上下文指针
 */
extern void show_all_register(struct arch_context *ptRegs);

/** @} */

/************************内联函数******************************/

/**
 * @defgroup TLBInstructions TLB指令操作
 * @{
 */

/**
 * @brief 清除TLB
 *
 * @details 执行tlbclr指令清除TLB
 */
static inline void tlbclr(void)
{
    __asm__ __volatile__("tlbclr");
}

/**
 * @brief 刷新TLB
 *
 * @details 执行tlbflush指令刷新TLB
 */
static inline void tlbflush(void)
{
    __asm__ __volatile__("tlbflush");
}

/**
 * @brief TLB探测
 *
 * @details 执行tlbsrch指令查找TLB项
 */
static inline void tlb_probe(void)
{
    __asm__ __volatile__("tlbsrch");
}

/**
 * @brief 读取TLB
 *
 * @details 执行tlbrd指令读取TLB项
 */
static inline void tlb_read(void)
{
    __asm__ __volatile__("tlbrd");
}

/**
 * @brief 索引方式写入TLB
 *
 * @details 执行tlbwr指令索引写入TLB
 */
static inline void tlb_write_indexed(void)
{
    __asm__ __volatile__("tlbwr");
}

/**
 * @brief 随机方式写入TLB
 *
 * @details 执行tlbfill指令随机写入TLB
 */
static inline void tlb_write_random(void)
{
    __asm__ __volatile__("tlbfill");
}

/** @} */

/**
 * @defgroup TLBInvalidate TLB无效化宏
 * @{
 */

/**
 * @brief TLB无效化宏（完整版）
 *
 * @param op   操作类型
 * @param info 信息参数
 * @param addr 地址参数
 */
#define invtlb(op, info, addr)    \
do    \
{    \
    asm volatile("invtlb %0, %1, %2\n\t"    \
                :    \
                : "i"(op), "r"(info), "r"(addr)    \
                : "memory");    \
} while (0)

/**
 * @brief TLB地址无效化宏
 *
 * @param op   操作类型
 * @param addr 地址参数
 */
#define invtlb_addr(op, info, addr)    \
do    \
{    \
    asm volatile("invtlb %0, $zero, %1\n\t"    \
                :    \
                : "i"(op), "r"(addr)    \
                : "memory");    \
} while (0)

/**
 * @brief TLB信息无效化宏
 *
 * @param op   操作类型
 * @param info 信息参数
 */
#define invtlb_info(op, info, addr)    \
do    \
{    \
    asm volatile("invtlb %0, %1, $zero\n\t"    \
                :    \
                : "i"(op), "r"(info)    \
                : "memory");    \
} while (0)

/**
 * @brief TLB全部无效化宏
 *
 * @param op 操作类型
 */
#define invtlb_all(op, info, addr)    \
do    \
{    \
        asm volatile("invtlb %0,$zero,$zero\n\t"    \
                    :    \
                    : "i"(op)    \
                    : "memory");    \
} while (0)

/** @} */

/************************C++兼容性******************************/
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif /* _TLB_H */
