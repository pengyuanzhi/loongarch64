/**
 * @file    arch_uaccess.c
 * @brief   LoongArch64用户空间访问
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件实现用户空间访问权限检查和数据复制
 *          - 用户空间地址验证
 *          - 内核空间地址验证
 *          - 页表权限检查
 *          - 安全的数据复制操作
 *
 * @note MISRA-C:2012 合规
 * @warning 用户空间访问必须严格验证权限
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

/*************************** 头文件包含 ****************************/
#include <commonTypes.h>
#include <errno.h>
#include <mmu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <system/types.h>
#include <ttosMM.h>
#include <uaccess.h>

#define KLOG_TAG "mmu_uaccess"
#include <klog.h>

/*************************** 宏定义 ****************************/
#define ACCESS_R 1
#define ACCESS_W 2
#define ACCESS_RW 4

/*************************** 类型定义 ****************************/

/*************************** 外部声明 ****************************/

/*************************** 前向声明 ****************************/
static uint64_t search_pgtable(virt_addr_t v_addr, phys_addr_t mmu_base);
static uint64_t page_entry_get(phys_addr_t pte, int level, int idx);
static bool is_page_dir(uint64_t ptable_entry);

/*************************** 模块变量 ****************************/

/*************************** 全局变量 ****************************/

/*************************** 函数实现 ****************************/

/**
 * @brief 用户空间访问权限检查
 *
 * @details 检查用户空间地址范围是否具有指定的访问权限
 *          验证地址范围是否在用户空间内
 *          遍历所有涉及的页表项，检查权限位
 *
 * @param user_addr 用户空间地址指针
 * @param n 要检查的字节数
 * @param flag 访问权限标志（ACCESS_R/ACCESS_W/ACCESS_RW）
 *
 * @return 检查成功返回true，失败返回false
 *
 * @note 此函数在系统调用和数据复制前调用
 * @warning 必须确保地址范围完全在用户空间内
 */
bool user_access_check(const void __user *user_addr, unsigned long n, int flag)
{
    void *addr_end;
    const void *addr_start = user_addr;
    unsigned long mmu_check_start;
    unsigned long mmu_check_size;
    void *user_space_start = (void *)USER_SPACE_START;
    void *user_space_end = (void *)(USER_SPACE_END);
    uint64_t ptable_entry = 0;
    uint64_t permission;
    if (!user_addr || !n)
    {
        return false;
    }
    addr_end = (void *)((char *)addr_start + n);
    if (addr_start > (void *)user_space_end || addr_start < user_space_start)
    {
        return false;
    }
    if (addr_end > (void *)user_space_end)
    {
        return false;
    }
    mmu_check_start = PAGE_SIZE_ALIGN((unsigned long)addr_start);
    mmu_check_size = roundup(n, PAGE_SIZE);
    while (mmu_check_size > 0)
    {
        ptable_entry = search_pgtable(mmu_check_start, csr_read64(LOONGARCH_CSR_PGDL));
        switch (flag)
        {
        case UACCESS_R:
            if (is_page_dir(ptable_entry))
                permission = ptable_entry;
            else
                permission = ptable_entry & (_PAGE_USER | __READABLE | _PAGE_PRESENT);
            break;
        case UACCESS_W:
            if (is_page_dir(ptable_entry))
                permission = ptable_entry;
            else
                permission = ptable_entry & (_PAGE_USER | __WRITEABLE | _PAGE_PRESENT);
            break;
        case UACCESS_RW:
            permission = ptable_entry & (_PAGE_USER | __WRITEABLE | _PAGE_PRESENT | __READABLE);
            break;
        default:
            return false;
        }
        if (!permission)
        {
            return false;
        }
        mmu_check_start += PAGE_SIZE;
        mmu_check_size -= PAGE_SIZE;
    }
    return true;
}

/**
 * @brief 内核空间访问权限检查
 *
 * @details 检查内核空间地址范围是否具有指定的访问权限
 *          遍历所有涉及的页表项，检查权限位
 *
 * @param kernel_addr 内核空间地址指针
 * @param n 要检查的字节数
 * @param flag 访问权限标志（ACCESS_R/ACCESS_W/ACCESS_RW）
 *
 * @return 检查成功返回true，失败返回false
 *
 * @note 内核空间使用PGDH（高地址页表）
 * @warning 内核访问失败通常表示严重错误
 */
bool kernel_access_check(const void *kernel_addr, unsigned long n, int flag)
{
    unsigned long mmu_check_start;
    unsigned long mmu_check_size;
    uint64_t ptable_entry = 0;
    uint64_t permission;
    if (!kernel_addr || !n)
    {
        return false;
    }
    mmu_check_start = PAGE_SIZE_ALIGN((unsigned long)kernel_addr);
    mmu_check_size = roundup(n, PAGE_SIZE);
    while (mmu_check_size > 0)
    {
        ptable_entry = search_pgtable(mmu_check_start, csr_read64(LOONGARCH_CSR_PGDH));
        switch (flag)
        {
        case UACCESS_R:
            if (is_page_dir(ptable_entry))
                permission = ptable_entry;
            else
                permission = (ptable_entry & __READABLE) && (ptable_entry & _PAGE_PRESENT);
            break;
        case UACCESS_W:
            if (is_page_dir(ptable_entry))
                permission = ptable_entry;
            else
                permission = (ptable_entry & __WRITEABLE) && (ptable_entry & _PAGE_PRESENT);
            break;
        default:
            return false;
        }
        if (!permission)
        {
            return false;
        }
        mmu_check_start += PAGE_SIZE;
        mmu_check_size -= PAGE_SIZE;
    }
    return true;
}

/**
 * @brief 获取页表项值
 *
 * @details 通过fix_map机制映射页表，然后读取指定索引的页表项
 *
 * @param pte 页表物理地址
 * @param level 页表级别
 * @param idx 页表项索引
 *
 * @return 页表项值
 */
static uint64_t page_entry_get(phys_addr_t pte, int level, int idx)
{
    uint64_t value;
    uint64_t *table = (uint64_t *)fix_map_set(FIX_MAP_PTABLE(level), pte, MT_KERNEL_MEM);
    value = table[idx];
    return value;
}

/**
 * @brief 搜索页表
 *
 * @details 遍历多级页表，查找虚拟地址对应的最终页表项
 *
 * @param v_addr 虚拟地址
 * @param mmu_base MMU基地址（页表物理地址）
 *
 * @return 最终页表项值，未找到返回0
 */
static uint64_t search_pgtable(virt_addr_t v_addr, phys_addr_t mmu_base)
{
    phys_addr_t pte = mmu_base;
    uint64_t value;
    int level;
    int off;
    for (level = 0; level < MAX_TABLE_LEVEL + 1; level++)
    {
        off = GET_TABLE_OFF(level, v_addr);
        value = page_entry_get(pte, level, off);
        if (value == (uint64_t)NULL)
        {
            /* entry无效 */
            return 0ULL;
        }
        if ((!(value & _PAGE_HUGE)) && (level < MAX_TABLE_LEVEL))
        {
            /* 如果不是最后一级，则取出表项，继续查找下一级 */
            pte = value & ENTRY_ADDRESS_MASK;
        }
        else
        {
            /* 最后一级页面，返回page */
            return value;
        }
    }
}

/**
 * @brief 判断页表项是否为页目录
 *
 * @details 检查页表项是否指向下一级页表而非物理页
 *
 * @param ptable_entry 页表项值
 *
 * @return 是页目录返回true，是物理页返回false
 */
static bool is_page_dir(uint64_t ptable_entry)
{
    if ((ptable_entry != NULL) && (ptable_entry & 0xffful))
        return false; /* page */
    else
        return true; /* page dir */
}
