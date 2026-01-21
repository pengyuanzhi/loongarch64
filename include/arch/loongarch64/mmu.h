/**
 * @file    mmu.h
 * @brief   LoongArch64 MMU配置接口
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件定义LoongArch64内存管理单元(MMU)相关接口
 *          - 地址空间布局定义
 *          - 页表级别配置
 *          - 固定映射区域管理
 *          - 页表描述符宏
 *          - FIXMAP位操作集合
 *
 * @note MISRA-C:2012 合规
 * @note 支持3级页表配置
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef __MMU_H__
#define __MMU_H__

/************************头 文 件******************************/
#ifndef ASM_USE
#include <commonTypes.h>
#include <system/const.h>
#include <stdint.h>
#include <cpu.h>
#include <pageTable.h>
#include <adrspace.h>

/************************宏 定 义******************************/

/**
 * @defgroup AddressWidth 地址宽度定义
 * @{
 */

/**
 * @brief 架构地址宽度（位）
 */
#define ARCH_ADDRESS_WIDTH_BITS    64UL

/**
 * @brief 虚拟地址宽度（位）
 *
 * @details 2K1000LA: 40位, 3A5000: 48位
 */
#define ARCH_VADDRESS_WIDTH_BITS    (g_vaBits)

/**
 * @brief 内核空间起始地址
 */
#define KERNEL_SPACE_START    UL(CONFIG_KERNEL_SPACE_START)

/**
 * @brief 用户空间起始地址
 */
#define USER_SPACE_START    UL(0x1000)

/**
 * @brief 用户空间结束地址
 */
#define USER_SPACE_END    (BIT(g_vaBits) - 1UL)

/**
 * @brief 内核空间结束地址
 */
#define KERNEL_SPACE_END    UINTPTR_MAX

/** @} */

/**
 * @defgroup ExternalVariables 外部变量
 * @{
 */

/**
 * @brief 虚拟映射基地址
 */
extern unsigned long vm_map_base;

/**
 * @brief 虚拟地址宽度（位）
 */
extern unsigned long g_vaBits;

/**
 * @brief 物理地址宽度（位）
 */
extern unsigned long g_paBits;

/** @} */

/**
 * @defgroup TableLevel 页表级别配置
 * @{
 */

/**
 * @brief 页表级掩码
 */
#define TABLE_LEVEL_MASK    UL(0x1ff)

/**
 * @brief 固定映射区域起始地址
 *
 * @details 最后一个页表可映射的区域
 */
#define FIX_MAP_START    (CSR_DMW1_BASE)

/**
 * @brief 固定映射索引定义
 */
#define FIX_MAP_SELF              0  /**< 自身映射 */
#define FIX_MAP_EARLYCON          1  /**< 早期控制台 */
#define FIX_MAP_PTABLE(level)     (2 + (level))  /**< 页表映射 */
#define FIX_MAP_POOL_START        (FIX_MAP_PTABLE(3) + 1)  /**< 池起始（最大级别为3） */

/**
 * @brief 页大小和级别定义
 */
#define FOURKB_SIZE_SHIFT    12U  /**< 4KB页大小移位 */
#define _512_SIZE_SHIFT      9U   /**< 512字节页表项移位 */
#define MAX_TABLE_LEVEL      3U   /**< 最大页表级别 */
#define MIN_MAP_LEVEL        0U   /**< 最小映射级别 */

/** @} */

/**
 * @defgroup EntryMask 页表项掩码
 * @{
 */

/**
 * @brief 页表项地址掩码
 *
 * @details 2K1000LA: PALEN=40, 3A5000: PALEN=48
 */
#define ENTRY_ADDRESS_MASK    0x0000fffffffff000ULL

/**
 * @brief 大页地址掩码
 */
#define ENTRY_HUGE_ADDRESS_MASK    0x0000ffffffffffe0ULL

/**
 * @brief 页表项属性掩码
 */
#define ENTRY_ATTRIB_MASK    0xe0000000000001ffULL

/** @} */

/**
 * @defgroup ZoneDef 内存区域定义
 * @{
 */

/**
 * @brief ZONE_NORMAL区域定义
 */
#define ZONE_NORMAL_START    KERNEL_SPACE_START    /**< 正常区域起始 */
#define ZONE_NORMAL_SIZE     (BIT(g_paBits))       /**< 正常区域大小 */
#define ZONE_NORMAL_END      (KERNEL_SPACE_START + ZONE_NORMAL_SIZE - 1UL)  /**< 正常区域结束 */

/**
 * @brief 内核虚拟地址宽度（位）
 */
#define ARCH_KERNEL_VADDRESS_WIDTH_BITS    (PGDIR_SHIFT + (PAGE_SIZE_SHIFT - 3U))

/**
 * @brief 正常内存属性
 */
#define MEM_NORMAL_ATTRS    (_PAGE_VALID | _PAGE_DIRTY | _PAGE_PRESENT | _PAGE_WRITE)

/** @} */

/************************类型定义******************************/

/**
 * @brief 页全局目录类型
 */
typedef struct
{
    unsigned long pgd;
} pgd_t;

/**
 * @brief 页表项类型
 */
typedef struct
{
    unsigned long pte;
} pte_t;

/**
 * @brief FIXMAP位集合类型
 *
 * @details 用于管理固定映射区域的位图
 */
typedef struct fixmap_set_t
{
    unsigned long __bits[512U / sizeof(long)];
} fixmap_set_t;

/************************外部变量******************************/
/************************外部函数******************************/

/************************内联函数******************************/

/**
 * @defgroup FixmapOps FIXMAP位操作
 * @{
 */

/**
 * @brief FIXMAP位操作宏（通用）
 */
#define __FIXMAP_op_S(i, size, set, op)    \
    ((i) / 8U >= (size)    \
     ? 0    \
     : (((unsigned long *)(set))[(i) / 8U / sizeof(long)] op (    \
         1UL << ((i) % (8U * sizeof(long))))))

#define FIXMAP_SET_S(i, size, set)    __FIXMAP_op_S(i, size, set, |=)
#define FIXMAP_CLR_S(i, size, set)    __FIXMAP_op_S(i, size, set, &= ~)
#define FIXMAP_ISSET_S(i, size, set)    __FIXMAP_op_S(i, size, set, &)

/**
 * @brief FIXMAP集合操作函数宏
 */
#define __FIXMAP_op_func_S(func, op)    \
    static __inline void __FIXMAP_##func##_S(size_t __size, fixmap_set_t *__dest,    \
                                           const fixmap_set_t *__src1,    \
                                           const fixmap_set_t *__src2)    \
    {    \
        size_t __i;    \
        for (__i = 0U; __i < __size / sizeof(long); __i++)    \
        {    \
            ((unsigned long *)__dest)[__i]    \
                = ((unsigned long *)__src1)[__i] op ((unsigned long *)__src2)[__i];    \
        }    \
    }

__FIXMAP_op_func_S(AND, &) __FIXMAP_op_func_S(OR, |) __FIXMAP_op_func_S(XOR, ^)

#define FIXMAP_AND_S(a, b, c, d)    __FIXMAP_AND_S(a, b, c, d)
#define FIXMAP_OR_S(a, b, c, d)     __FIXMAP_OR_S(a, b, c, d)
#define FIXMAP_XOR_S(a, b, c, d)    __FIXMAP_XOR_S(a, b, c, d)

#define FIXMAP_ZERO_S(size, set)         memset(set, 0, size)
#define FIXMAP_EQUAL_S(size, set1, set2)    (!memcmp(set1, set2, size))

/**
 * @brief FIXMAP操作包装宏
 */
#define FIXMAP_SET(i, set)    FIXMAP_SET_S(i, sizeof(fixmap_set_t), set)
#define FIXMAP_CLR(i, set)    FIXMAP_CLR_S(i, sizeof(fixmap_set_t), set)
#define FIXMAP_ISSET(i, set)    FIXMAP_ISSET_S(i, sizeof(fixmap_set_t), set)
#define FIXMAP_AND(d, s1, s2)    FIXMAP_AND_S(sizeof(fixmap_set_t), d, s1, s2)
#define FIXMAP_OR(d, s1, s2)     FIXMAP_OR_S(sizeof(fixmap_set_t), d, s1, s2)
#define FIXMAP_XOR(d, s1, s2)    FIXMAP_XOR_S(sizeof(fixmap_set_t), d, s1, s2)
#define FIXMAP_ZERO(set)      FIXMAP_ZERO_S(sizeof(fixmap_set_t), set)
#define FIXMAP_EQUAL(s1, s2)    FIXMAP_EQUAL_S(sizeof(fixmap_set_t), s1, s2)

/** @} */

/**
 * @brief 获取映射级别
 *
 * @details 根据虚拟地址、物理地址和大小确定最佳映射级别
 *
 * @param va   虚拟地址
 * @param pa   物理地址
 * @param size 映射大小
 *
 * @return 返回最佳映射级别
 */
static inline int GET_MAP_LEVEL(uint64_t va, uint64_t pa, size_t size)
{
    int level;

    for (level = MIN_MAP_LEVEL; level < MAX_TABLE_LEVEL; level++)
    {
        if (IS_LEVEL_ALIGN(level, va) && IS_LEVEL_ALIGN(level, pa)
            && IS_LEVEL_ALIGN(level, size))
        {
            return level;
        }
    }
    return level;
}

/**
 * @defgroup PageDescriptor 页表描述符宏
 * @{
 */

/**
 * @brief 块描述符（用于大页映射）
 */
#define BLOCK_DESC(attr, addr)    \
    (((attr) & ENTRY_ATTRIB_MASK) | (((uint64_t)addr) & ENTRY_HUGE_ADDRESS_MASK)    \
     | _PAGE_HUGE | _PAGE_PRESENT | __READABLE | __WRITEABLE | _PAGE_HGLOBAL)

/**
 * @brief 页描述符（用于普通页映射）
 */
#define PAGE_DESC(attr, addr)    \
    (((attr) & ENTRY_ATTRIB_MASK) | (((uint64_t)addr) & ENTRY_ADDRESS_MASK)    \
     | _PAGE_PRESENT | __READABLE | __WRITEABLE)

/**
 * @brief 表描述符（扩展版，带属性）
 */
#define TABLE_DESC_EX(attr, next_table_point)    \
    (((attr) & ENTRY_ATTRIB_MASK) | ((next_table_point) & ENTRY_ADDRESS_MASK))

/**
 * @brief 表描述符（指向下一级页表）
 */
#define TABLE_DESC(next_table_point)    \
    TABLE_DESC_EX(0ULL, ((uint64_t)next_table_point))

/** @} */

/************************C++兼容性******************************/
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif

#endif /* __MMU_H__ */
