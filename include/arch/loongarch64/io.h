/**
 * @file    io.h
 * @brief   LoongArch64 I/O操作接口
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件定义LoongArch64 I/O操作相关接口
 *          - 基本I/O读写操作
 *          - I/O内存屏障
 *          - 字节序转换
 *          - 位操作宏
 *          - I/O重映射
 *          - 等待位操作
 *
 * @note MISRA-C:2012 合规
 * @note 当前为通用实现，未考虑特定大小端
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef _IO_H
#define _IO_H

/************************头 文 件******************************/
#include <barrier.h>
#include <system/types.h>
#include <errno.h>
#include <stdbool.h>
#include <adrspace.h>
#include <pageTable.h>

/************************宏 定 义******************************/

/**
 * @defgroup BasicIO 基本I/O操作
 * @{
 */

/**
 * @brief 体系架构通用I/O读接口
 *
 * @details 直接从I/O地址读取数据（volatile确保不被优化）
 */
#define arch_readb(a)    (*(volatile unsigned char *)(a))
#define arch_readw(a)    (*(volatile unsigned short *)(a))
#define arch_readl(a)    (*(volatile unsigned int *)(a))
#define arch_readq(a)    (*(volatile unsigned long long *)(a))

/**
 * @brief 体系架构通用I/O写接口
 *
 * @details 直接向I/O地址写入数据（volatile确保不被优化）
 */
#define arch_writeb(v, a)    (*(volatile unsigned char *)(a) = (v))
#define arch_writew(v, a)    (*(volatile unsigned short *)(a) = (v))
#define arch_writel(v, a)    (*(volatile unsigned int *)(a) = (v))
#define arch_writeq(v, a)    (*(volatile unsigned long long *)(a) = (v))

/** @} */

/**
 * @defgroup IOBarriers I/O屏障
 * @{
 */

/**
 * @brief 编译器屏障
 *
 * @details 防止编译器重排内存访问
 */
#define barrier()    __asm__ __volatile__("" : : : "memory")

/**
 * @brief I/O读屏障
 */
#define iormb()    rmb()

/**
 * @brief I/O写屏障
 */
#define iowmb()    wmb()

/** @} */

/**
 * @defgroup StandardIO 标准I/O读写
 * @{
 */

/**
 * @brief 通用I/O读接口（带读屏障）
 *
 * @details 从I/O地址读取8位数据并插入读屏障
 */
#define readb(c)    \
    ({    \
        unsigned char v = arch_readb(c);    \
        iormb();    \
        v;    \
    })

/**
 * @brief 通用I/O读接口（带读屏障）
 *
 * @details 从I/O地址读取16位数据并插入读屏障
 */
#define readw(c)    \
    ({    \
        unsigned short v = arch_readw(c);    \
        iormb();    \
        v;    \
    })

/**
 * @brief 通用I/O读接口（带读屏障）
 *
 * @details 从I/O地址读取32位数据并插入读屏障
 */
#define readl(c)    \
    ({    \
        unsigned int v = arch_readl(c);    \
        iormb();    \
        v;    \
    })

/**
 * @brief 通用I/O读接口（带读屏障）
 *
 * @details 从I/O地址读取64位数据并插入读屏障
 */
#define readq(c)    \
    ({    \
        unsigned long long v = arch_readq(c);    \
        iormb();    \
        v;    \
    })

/**
 * @brief 通用I/O写接口（带写屏障）
 *
 * @details 向I/O地址写入8位数据（前插写屏障）
 */
#define writeb(v, c)    \
    ({    \
        iowmb();    \
        arch_writeb((v), (c));    \
    })

/**
 * @brief 通用I/O写接口（带写屏障）
 *
 * @details 向I/O地址写入16位数据（前插写屏障）
 */
#define writew(v, c)    \
    ({    \
        iowmb();    \
        arch_writew((v), (c));    \
    })

/**
 * @brief 通用I/O写接口（带写屏障）
 *
 * @details 向I/O地址写入32位数据（前插写屏障）
 */
#define writel(v, c)    \
    ({    \
        iowmb();    \
        arch_writel((v), (c));    \
    })

/**
 * @brief 通用I/O写接口（带写屏障）
 *
 * @details 向I/O地址写入64位数据（前插写屏障）
 */
#define writeq(v, c)    \
    ({    \
        iowmb();    \
        arch_writeq((v), (c));    \
    })

/** @} */

/**
 * @defgroup ByteSwap 字节序转换
 * @{
 */

/**
 * @brief 32位字节交换（大端序）
 */
#define _swab32(x)    \
    ((u32)(    \
        (((u32)(x) & (u32)0x000000ffUL) << 24) |    \
        (((u32)(x) & (u32)0x0000ff00UL) <<  8) |    \
        (((u32)(x) & (u32)0x00ff0000UL) >>  8) |    \
        (((u32)(x) & (u32)0xff000000UL) >> 24) ))

/**
 * @brief 16位字节交换（大端序）
 */
#define _swab16(x)    \
    ((u16)(    \
        (((u16)(x) & (u16)0x00ffUL) << 8) |    \
        (((u16)(x) & (u16)0xff00UL) >>  8) ))

/**
 * @brief CPU序到大端序转换
 */
#define cpu_to_be32(x)    _swab32(x)
#define be32_to_cpu(x)    _swab32(x)

/**
 * @brief CPU序到大端序转换（16位）
 */
#define cpu_to_be16(x)    _swab16(x)
#define be16_to_cpu(x)    _swab16(x)

/** @} */

/**
 * @defgroup EndianIO 大小端I/O操作
 * @{
 */

/**
 * @brief 大端序I/O读模板
 */
#define in_arch(type, endian, a)    endian##_to_cpu(read##type(a))

/**
 * @brief 大端序I/O写模板
 */
#define out_arch(type, endian, a, v)    write##type(cpu_to_##endian(v), a)

/**
 * @brief 大端32位I/O操作
 */
#define out_be32(a, v)    out_arch(l, be32, a, v)
#define in_be32(a)    in_arch(l, be32, a)

/**
 * @brief 大端16位I/O操作
 */
#define out_be16(a, v)    out_arch(l, be16, a, v)
#define in_be16(a)    in_arch(l, be16, a)

/**
 * @brief 小端32位I/O操作
 */
#define out_le32(a, v)    writel(v, a)
#define in_le32(a)    readl(a)

/**
 * @brief 小端16位I/O操作
 */
#define out_le16(a, v)    writew(v, a)
#define in_le16(a)    readw(a)

/**
 * @brief 8位I/O操作
 */
#define out_8(a, v)    writeb(v, a)
#define in_8(a)    readb(a)

/** @} */

/**
 * @defgroup BitOps 位操作宏
 * @{
 */

/**
 * @brief 清除位
 */
#define clrbits(type, addr, clear)    \
    out_##type((addr), in_##type(addr) & ~(clear))

/**
 * @brief 设置位
 */
#define setbits(type, addr, set)    \
    out_##type((addr), in_##type(addr) | (set))

/**
 * @brief 清除并设置位
 */
#define clrsetbits(type, addr, clear, set)    \
    out_##type((addr), (in_##type(addr) & ~(clear)) | (set))

/**
 * @brief 小端32位位操作
 */
#define clrbits_le32(addr, clear)    clrbits(le32, addr, clear)
#define setbits_le32(addr, set)    setbits(le32, addr, set)
#define clrsetbits_le32(addr, clear, set)    clrsetbits(le32, addr, clear, set)

/**
 * @brief 小端16位位操作
 */
#define clrbits_le16(addr, clear)    clrbits(le16, addr, clear)
#define setbits_le16(addr, set)    setbits(le16, addr, set)
#define clrsetbits_le16(addr, clear, set)    clrsetbits(le16, addr, clear, set)

/**
 * @brief 大端32位位操作
 */
#define clrbits_be32(addr, clear)    clrbits(be32, addr, clear)
#define setbits_be32(addr, set)    setbits(be32, addr, set)
#define clrsetbits_be32(addr, clear, set)    clrsetbits(be32, addr, clear, set)

/**
 * @brief 8位位操作
 */
#define clrbits_8(addr, clear)    clrbits(8, addr, clear)
#define setbits_8(addr, set)    setbits(8, addr, set)
#define clrsetbits_8(addr, clear, set)    clrsetbits(8, addr, clear, set)

/** @} */

/************************类型定义******************************/
/************************外部变量******************************/
/************************外部函数******************************/

/**
 * @defgroup IOFunctions I/O功能函数
 * @{
 */

/**
 * @brief 微秒级延时
 *
 * @param usecs 微秒数
 *
 * @return 成功返回0
 */
int usleep(unsigned usecs);

/**
 * @brief 等待位操作完成（小端32位）
 *
 * @details 等待寄存器的指定位满足条件
 *
 * @param reg        寄存器地址
 * @param mask       位掩码
 * @param set        等置位(true)或等待清零(false)
 * @param timeout_ms 超时时间（毫秒）
 * @param breakable  是否可中断
 *
 * @return 成功返回0，超时返回-ETIMEDOUT
 */
static inline int wait_for_bit_le32(const void *reg,
                                    const u32 mask,
                                    const bool set,
                                    const unsigned int timeout_ms,
                                    const bool breakable)
{
    u32 val;
    u32 time = 0U;
    u64 timeout_us = (u64)timeout_ms * 1000000ULL;

    (void)breakable; /* 未使用参数 */

    for (;;)
    {
        val = readl(reg);

        if (!set)
        {
            val = ~val;
        }

        if ((val & mask) == mask)
        {
            return 0;
        }

        if (time > timeout_us)
        {
            break;
        }
        (void)usleep(1U);
        time++;
    }
    return -ETIMEDOUT;
}

/**
 * @brief I/O内存重映射（带保护属性）
 *
 * @details 将物理地址映射到虚拟地址空间，并指定缓存属性
 *
 * @param offset   物理地址偏移
 * @param size     映射大小
 * @param prot_val 保护属性
 *                  - _CACHE_CC: 可缓存
 *                  - _CACHE_SUC: 不可缓存
 *                  - _CACHE_WUC: 写合并
 *
 * @return 成功返回虚拟地址指针，失败返回NULL
 */
static inline void *ioremap_prot(phys_addr_t offset, unsigned long size,
                                 unsigned long prot_val)
{
    (void)size; /* 未使用参数 */

    switch (prot_val)
    {
        case _CACHE_CC:
            return (void *)(unsigned long)(CACHE_BASE + offset);
        case _CACHE_SUC:
            return (void *)(unsigned long)(UNCACHE_BASE + offset);
        case _CACHE_WUC:
            return (void *)(unsigned long)(WRITECOMBINE_BASE + offset);
        default:
            return NULL;
    }
}

/** @} */

/************************内联函数******************************/
/************************C++兼容性******************************/
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif /* _IO_H */
