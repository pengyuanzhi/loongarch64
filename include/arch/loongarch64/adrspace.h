/**
 * @file    adrspace.h
 * @brief   LoongArch64地址空间定义
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 本文件定义LoongArch64地址空间布局相关常量和宏
 *          - I/O空间映射基地址
 *          - 物理地址转换宏
 *          - 内核/用户空间范围
 *          - PCI/ISA I/O空间定义
 *
 * @note MISRA-C:2012 合规
 * @note LoongArch64使用直接映射窗口（DMW）进行地址转换
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef __TTOS_ADRSPACE_H__
#define __TTOS_ADRSPACE_H__

/************************头 文 件******************************/
#include <cpu.h>
#include <mmu.h>

/************************宏 定 义******************************/

/**
 * @defgroup AddressMappingBase 地址映射基地址
 * @{
 */

/**
 * @brief I/O空间基地址
 *
 * @details 默认使用DMW0作为I/O空间映射
 */
#ifndef IO_BASE
#define IO_BASE CSR_DMW0_BASE
#endif

/**
 * @brief 可缓存空间基地址
 *
 * @details 使用DMW1作为可缓存映射
 */
#ifndef CACHE_BASE
#define CACHE_BASE CSR_DMW1_BASE
#endif

/**
 * @brief 不可缓存空间基地址
 *
 * @details 使用DMW0作为不可缓存映射
 */
#ifndef UNCACHE_BASE
#define UNCACHE_BASE CSR_DMW0_BASE
#endif

/**
 * @brief 写合并空间基地址
 *
 * @details 使用DMW2作为写合并映射
 */
#ifndef WRITECOMBINE_BASE
#define WRITECOMBINE_BASE CSR_DMW2_BASE
#endif

/** @} */

/**
 * @defgroup AddressConv 地址转换宏
 * @{
 */

/**
 * @brief 物理地址掩码
 *
 * @details 根据DMW_PABITS生成物理地址掩码
 */
#define TO_PHYS_MASK ((1ULL << DMW_PABITS) - 1ULL)

/**
 * @brief 高内存起始地址
 *
 * @details 高于此物理地址的内存被视为高内存
 */
#ifndef HIGHMEM_START
#define HIGHMEM_START (_UL(1) << _UL(DMW_PABITS))
#endif

/**
 * @brief 转换为物理地址
 *
 * @param x 虚拟地址
 *
 * @return 返回物理地址
 */
#define TO_PHYS(x) ((x) & TO_PHYS_MASK)

/**
 * @brief 转换为可缓存地址
 *
 * @param x 物理地址
 *
 * @return 返回可缓存虚拟地址
 */
#define TO_CACHE(x) (CACHE_BASE | ((x) & TO_PHYS_MASK))

/**
 * @brief 转换为不可缓存地址
 *
 * @param x 物理地址
 *
 * @return 返回不可缓存虚拟地址
 */
#define TO_UNCACHE(x) (UNCACHE_BASE | ((x) & TO_PHYS_MASK))

/** @} */

/**
 * @defgroup MemoryMap 内存映射
 * @{
 */

/**
 * @brief 页偏移
 *
 * @details 物理偏移的缓存映射虚拟地址
 */
#ifndef PAGE_OFFSET
#define PAGE_OFFSET (CACHE_BASE + PHYS_OFFSET)
#endif

/**
 * @brief 固定映射区域顶端地址
 */
#ifndef FIXADDR_TOP
#define FIXADDR_TOP ((unsigned long)(long)(int)0xfffe0000UL)
#endif

/** @} */

/**
 * @defgroup AddrType 地址类型定义
 * @{
 */

#ifdef ASM_USE
#define _ATYPE_
#define _ATYPE32_
#define _ATYPE64_
#else
#define _ATYPE_ __PTRDIFF_TYPE__
#define _ATYPE32_ int
#define _ATYPE64_ long
#endif

/**
 * @brief LoongArch 32/64位地址空间类型
 */
#ifdef ASM_USE
#define _ACAST32_
#define _ACAST64_
#else
#define _ACAST32_ (_ATYPE_)(_ATYPE32_) /**< 必要时扩展 */
#define _ACAST64_ (_ATYPE64_) /**< 不缩小 */
#endif

/** @} */

/**
 * @defgroup AddressRanges 地址范围定义
 * @{
 */

#ifdef CONFIG_32BIT
#define UVRANGE 0x00000000U /**< 用户虚拟范围 */
#define KPRANGE0 0x80000000U /**< 内核物理范围0 */
#define KPRANGE1 0xa0000000U /**< 内核物理范围1 */
#define KVRANGE 0xc0000000U /**< 内核虚拟范围 */
#else
#define XUVRANGE _CONST64_(0x0000000000000000) /**< 扩展用户虚拟范围 */
#define XSPRANGE _CONST64_(0x4000000000000000) /**< 扩展系统物理范围 */
#define XKPRANGE _CONST64_(0x8000000000000000) /**< 扩展内核物理范围 */
#define XKVRANGE _CONST64_(0xc000000000000000) /**< 扩展内核虚拟范围 */
#endif

/**
 * @brief 获取物理地址
 *
 * @param a KPRANGEx / XKPRANGE地址
 *
 * @return 返回物理地址
 */
#define PHYSADDR(a) ((_ACAST64_(a)) & TO_PHYS_MASK)

/** @} */

/**
 * @defgroup IOSpace I/O空间映射
 * @{
 */

/**
 * @brief LoongArch I/O端口映射
 *
 * @details I/O端口布局：
 *          |         ....          |
 *          |-----------------------|
 *          | pci io ports(16K~32M) |
 *          |-----------------------|
 *          | isa io ports(0  ~16K) |
 *          PCI_IOBASE ->|-----------------------|
 *          |         ....          |
 */

/**
 * @brief PCI I/O基地址
 */
#define PCI_IOBASE ((vm_map_base + (2UL * PAGE_SIZE)))

/**
 * @brief PCI I/O空间大小
 */
#define PCI_IOSIZE SZ_32M

/**
 * @brief ISA I/O空间大小
 */
#define ISA_IOSIZE SZ_16K

/**
 * @brief I/O空间限制
 */
#define IO_SPACE_LIMIT (PCI_IOSIZE - 1UL)

/**
 * @brief 物理链接内核地址
 */
#define PHYS_LINK_KADDR PHYSADDR(CONFIG_KERNEL_SPACE_START)

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

#endif /* __TTOS_ADRSPACE_H__ */
