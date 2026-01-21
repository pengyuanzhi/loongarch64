/**
 * @file    pageTable.h
 * @brief   LoongArch64页表定义
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件定义LoongArch64页表相关常量和宏
 *          - 多级页表配置
 *          - 页表项位定义
 *          - 缓存属性
 *          - 页大小定义
 *          - 页表级别宏
 *
 * @note MISRA-C:2012 合规
 * @note 支持2/3/4级页表配置
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef __PAGETABLE_H__
#define __PAGETABLE_H__

/************************头 文 件******************************/

/************************宏 定 义******************************/

/**
 * @defgroup PageTableLevel 页表级别配置
 * @{
 */

/**
 * @brief 根据页表级别配置各级移位和掩码
 */
#if CONFIG_PGTABLE_LEVELS == 2

#define PGDIR_SHIFT (PAGE_SIZE_SHIFT + (PAGE_SIZE_SHIFT - 3))

#elif CONFIG_PGTABLE_LEVELS == 3

#define PMD_SHIFT (PAGE_SIZE_SHIFT + (PAGE_SIZE_SHIFT - 3))
#define PMD_SIZE (1UL << PMD_SHIFT)
#define PMD_MASK (~(PMD_SIZE - 1UL))
#define PGDIR_SHIFT (PMD_SHIFT + (PAGE_SIZE_SHIFT - 3))

#elif CONFIG_PGTABLE_LEVELS == 4

#define PMD_SHIFT (PAGE_SIZE_SHIFT + (PAGE_SIZE_SHIFT - 3))
#define PMD_SIZE (1UL << PMD_SHIFT)
#define PMD_MASK (~(PMD_SIZE - 1UL))
#define PUD_SHIFT (PMD_SHIFT + (PAGE_SIZE_SHIFT - 3))
#define PUD_SIZE (1UL << PUD_SHIFT)
#define PUD_MASK (~(PUD_SIZE - 1UL))
#define PGDIR_SHIFT (PUD_SHIFT + (PAGE_SIZE_SHIFT - 3))

#endif

/**
 * @brief 每级页表项数量
 */
#define PTRS_PER_PGD (PAGE_SIZE_SHIFT >> 3)

#if CONFIG_PGTABLE_LEVELS > 3
#define PTRS_PER_PUD (PAGE_SIZE_SHIFT >> 3)
#endif

#if CONFIG_PGTABLE_LEVELS > 2
#define PTRS_PER_PMD (PAGE_SIZE_SHIFT >> 3)
#endif

#define PTRS_PER_PTE (PAGE_SIZE_SHIFT >> 3)

/** @} */

/**
 * @defgroup PageTableBits 页表项位定义
 * @{
 */

/**
 * @brief 页表项基础位
 */
#define PAGE_V (1UL << 0)   /**< 有效位 */
#define PAGE_D (1UL << 1)   /**< 脏位 */
#define PAGE_P (1UL << 7)   /**< 存在位 */
#define PAGE_W (1UL << 8)   /**< 写使能位 */
#define PAGE_NR (1UL << 61) /**< 保留位 */
#define PAGE_NX (1UL << 62) /**< 不可执行位 */

/**
 * @brief 页表项位移位定义
 */
#define _PAGE_VALID_SHIFT 0            /**< 有效位移位 */
#define _PAGE_ACCESSED_SHIFT 0         /**< 访问位移位（复用Valid） */
#define _PAGE_DIRTY_SHIFT 1            /**< 脏位移位 */
#define _PAGE_PLV_SHIFT 2              /**< 特权级位移位（2-3，2位） */
#define _CACHE_SHIFT 4                 /**< 缓存属性位移位（4-5，2位） */
#define _PAGE_GLOBAL_SHIFT 6           /**< 全局位移位 */
#define _PAGE_HUGE_SHIFT 6             /**< 大页位移位（PMD位） */
#define _PAGE_PRESENT_SHIFT 7          /**< 存在位移位 */
#define _PAGE_WRITE_SHIFT 8            /**< 写使能位移位 */
#define _PAGE_MODIFIED_SHIFT 9         /**< 修改位移位 */
#define _PAGE_PROTNONE_SHIFT 10        /**< 保护无位移位 */
#define _PAGE_SPECIAL_SHIFT 11         /**< 特殊页位移位 */
#define _PAGE_HGLOBAL_SHIFT 12         /**< 全局大页位移位（PMD位） */
#define _PAGE_PFN_SHIFT 12             /**< 页帧号位移位 */
#define _PAGE_SWP_EXCLUSIVE_SHIFT 23   /**< 交换独占位移位 */
#define _PAGE_PFN_END_SHIFT 48         /**< 页帧号结束位移位 */
#define _PAGE_DEVMAP_SHIFT 59          /**< 设备映射位移位 */
#define _PAGE_PRESENT_INVALID_SHIFT 60 /**< 存在但无效位移位 */
#define _PAGE_NO_READ_SHIFT 61         /**< 不可读位移位 */
#define _PAGE_NO_EXEC_SHIFT 62         /**< 不可执行位移位 */
#define _PAGE_RPLV_SHIFT 63            /**< 复位特权级位移位 */

/**
 * @brief 软件使用的页表位
 */
#define _PAGE_PRESENT (_ULCAST_(1) << _PAGE_PRESENT_SHIFT)
#define _PAGE_PRESENT_INVALID (_ULCAST_(1) << _PAGE_PRESENT_INVALID_SHIFT)
#define _PAGE_WRITE (_ULCAST_(1) << _PAGE_WRITE_SHIFT)
#define _PAGE_ACCESSED (_ULCAST_(1) << _PAGE_ACCESSED_SHIFT)
#define _PAGE_MODIFIED (_ULCAST_(1) << _PAGE_MODIFIED_SHIFT)
#define _PAGE_PROTNONE (_ULCAST_(1) << _PAGE_PROTNONE_SHIFT)
#define _PAGE_SPECIAL (_ULCAST_(1) << _PAGE_SPECIAL_SHIFT)
#define _PAGE_DEVMAP (_ULCAST_(1) << _PAGE_DEVMAP_SHIFT)
#define _PAGE_SWP_EXCLUSIVE (_ULCAST_(1) << _PAGE_SWP_EXCLUSIVE_SHIFT)

/**
 * @brief TLB硬件使用的页表位（放置在EntryLo*中）
 */
#define _PAGE_VALID (_ULCAST_(1) << _PAGE_VALID_SHIFT)
#define _PAGE_DIRTY (_ULCAST_(1) << _PAGE_DIRTY_SHIFT)
#define _PAGE_PLV (_ULCAST_(3) << _PAGE_PLV_SHIFT)
#define _PAGE_GLOBAL (_ULCAST_(1) << _PAGE_GLOBAL_SHIFT)
#define _PAGE_HUGE (_ULCAST_(1) << _PAGE_HUGE_SHIFT)
#define _PAGE_HGLOBAL (_ULCAST_(1) << _PAGE_HGLOBAL_SHIFT)
#define _PAGE_NO_READ (_ULCAST_(1) << _PAGE_NO_READ_SHIFT)
#define _PAGE_NO_EXEC (_ULCAST_(1) << _PAGE_NO_EXEC_SHIFT)
#define _PAGE_RPLV (_ULCAST_(1) << _PAGE_RPLV_SHIFT)
#define _CACHE_MASK (_ULCAST_(3) << _CACHE_SHIFT)

/**
 * @brief 特权级页表位
 */
#define _PAGE_USER (PLV_USER << _PAGE_PLV_SHIFT)
#define _PAGE_KERN (PLV_KERN << _PAGE_PLV_SHIFT)

/**
 * @brief 页帧号掩码
 */
#define _PFN_MASK (~((_ULCAST_(1) << (PFN_PTE_SHIFT)) - 1UL) & ((_ULCAST_(1) << (_PAGE_PFN_END_SHIFT)) - 1UL))

/** @} */

/**
 * @defgroup CacheAttr 缓存属性
 * @{
 */

/**
 * @brief 缓存属性定义
 */
#ifndef _CACHE_SUC
#define _CACHE_SUC (0 << _CACHE_SHIFT) /**< 强序不可缓存 */
#endif

#ifndef _CACHE_CC
#define _CACHE_CC (1 << _CACHE_SHIFT) /**< 一致性缓存 */
#endif

#ifndef _CACHE_WUC
#define _CACHE_WUC (2 << _CACHE_SHIFT) /**< 弱序不可缓存 */
#endif

/**
 * @brief 可读写属性
 */
#define __READABLE _PAGE_VALID
#define __WRITEABLE (_PAGE_DIRTY | _PAGE_WRITE)

/** @} */

/**
 * @defgroup PageSize 页大小定义
 * @{
 */

/**
 * @brief PageSize寄存器值
 */
#define PS_4K 0x0000000cUL
#define PS_8K 0x0000000dUL
#define PS_16K 0x0000000eUL
#define PS_32K 0x0000000fUL
#define PS_64K 0x00000010UL
#define PS_128K 0x00000011UL
#define PS_256K 0x00000012UL
#define PS_512K 0x00000013UL
#define PS_1M 0x00000014UL
#define PS_2M 0x00000015UL
#define PS_4M 0x00000016UL
#define PS_8M 0x00000017UL
#define PS_16M 0x00000018UL
#define PS_32M 0x00000019UL
#define PS_64M 0x0000001aUL
#define PS_128M 0x0000001bUL
#define PS_256M 0x0000001cUL
#define PS_512M 0x0000001dUL
#define PS_1G 0x0000001eUL

/**
 * @brief 页表描述符属性
 */
#define PAGE_TABLE (_PAGE_PRESENT | __READABLE | __WRITEABLE | _PAGE_KERN | _CACHE_CC)

/**
 * @brief 表级掩码
 */
#define TABLE_LEVEL_MASK UL(0x1ff)

/**
 * @brief 页大小和级别相关定义
 */
#define PAGE_4K_SIZE_SHIFT 12U
#define _512_SIZE_SHIFT 9U
#define MAX_TABLE_LEVEL 3U

/**
 * @brief 级别相关宏
 */
#define LEVEL_FINAL_SIZE_SHIFT PAGE_4K_SIZE_SHIFT
#define LEVEL_SIZE_SHIFT(level) (LEVEL_FINAL_SIZE_SHIFT + (MAX_TABLE_LEVEL - (level)) * _512_SIZE_SHIFT)
#define LEVEL_SIZE(level) (UL(0x01) << LEVEL_SIZE_SHIFT(level))
#define LEVEL_SIZE_MASK(level) (LEVEL_SIZE(level) - 1UL)
#define LEVEL_SIZE_ALIGN(level, addr) ((addr) & (~LEVEL_SIZE_MASK(level)))

/**
 * @brief 级别对齐和偏移宏
 */
#define IS_LEVEL_ALIGN(level, addr) (((addr) & LEVEL_SIZE_MASK(level)) == U(0))
#define GET_TABLE_OFF(level, addr) (((addr) >> LEVEL_SIZE_SHIFT(level)) & TABLE_LEVEL_MASK)

/**
 * @brief 页大小宏
 */
#define PAGE_SIZE LEVEL_SIZE(MAX_TABLE_LEVEL)
#define PAGE_SIZE_SHIFT LEVEL_SIZE_SHIFT(MAX_TABLE_LEVEL)
#define PAGE_SIZE_MASK LEVEL_SIZE_MASK(MAX_TABLE_LEVEL)
#define PAGE_SIZE_ALIGN(addr) LEVEL_SIZE_ALIGN(MAX_TABLE_LEVEL, addr)
#define PAGE_MASK (~(PAGE_SIZE - 1UL))

/**
 * @brief 大页（Huge Page）定义
 */
#define HPAGE_SHIFT (PAGE_SIZE_SHIFT + PAGE_SIZE_SHIFT - 3U)
#define HPAGE_SIZE (_AC(1, UL) << HPAGE_SHIFT)
#define HPAGE_MASK (~(HPAGE_SIZE - 1UL))
#define HUGETLB_PAGE_ORDER (HPAGE_SHIFT - PAGE_SIZE_SHIFT)

/**
 * @brief 默认页大小
 */
#define PS_DEFAULT_SIZE PS_4K

/**
 * @brief 默认大页大小
 */
#define PS_HUGE_SIZE PS_1M

/** @} */

/************************类型定义******************************/
/************************外部变量******************************/
/************************外部函数******************************/
/************************内联函数******************************/
/************************C++兼容性******************************/
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif /* __PAGETABLE_H__ */
