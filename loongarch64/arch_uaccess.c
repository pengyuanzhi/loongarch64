#include <commonTypes.h>
/**
 * @file    arch/x86_64/arch_uaccess.c
 * @author  maoyz
 * @brief
 * @version 3.0.0
 * @date    2025-02-21
 *
 * 8c83d7de 2024-07-02 移除头文件路径中的linux
 * ac006b61 2024-07-02 移除一级ttos目录
 * b041d869 2024-05-15 格式化代码并处理一些头文件依赖问题
 * 5a4fab1e 2024-05-13 修复uaccess
 * db6c3f60 2024-04-24 添加copy_from_user copy_to_user代码实现(未验证)
 *
 * 科东(广州)软件科技有限公司 版权所有
 * @copyright Copyright (C) 2023 Intewell Inc. All Rights Reserved.
 */
/************************头 文 件******************************/
#include <errno.h>
#include <system/types.h>
#include <mmu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <uaccess.h>
#include <ttosMM.h>
#define KLOG_TAG "mmu_uaccess"
#include <klog.h>
/************************宏 定 义******************************/
#define ACCESS_R  1
#define ACCESS_W  2
#define ACCESS_RW 4
/************************类型定义******************************/
/************************外部声明******************************/
/************************前向声明******************************/
static uint64_t search_pgtable(virt_addr_t v_addr,phys_addr_t mmu_base);
static uint64_t page_entry_get(phys_addr_t pte, int level, int idx);
static bool is_page_dir(uint64_t ptable_entry);
/************************模块变量******************************/
/************************全局变量******************************/
/************************函数实现******************************/
/**
 * @brief
 *    用户空间访问权限检查
 * @param[in] user_addr 用户态地址
 * @param[in] n 要检查的字节数
 * @param[in] flag 要检查的权限
 * @retval
 *   true 检查成功
 *   false 检查失败
 */
bool user_access_check (const void __user *user_addr, unsigned long n, int flag)
{
    void         *addr_end;
    const void   *addr_start = user_addr;
    unsigned long mmu_check_start;
    unsigned long mmu_check_size;
    void         *user_space_start = (void *)USER_SPACE_START;
    void         *user_space_end   = (void *)(USER_SPACE_END);
    uint64_t     ptable_entry = 0;
    uint64_t         permission;
    if (!user_addr || !n)
    {
        return false;
    }addr_end = (void *)((char *)addr_start + n);
    if (addr_start > (void *)user_space_end || addr_start < user_space_start)
    {
        return false;
    }if (addr_end > (void *)user_space_end)
    {
        return false;
    }mmu_check_start = PAGE_SIZE_ALIGN ((unsigned long)addr_start);
    mmu_check_size  = roundup (n, PAGE_SIZE);
    while (mmu_check_size > 0)
    {
        ptable_entry = search_pgtable(mmu_check_start,csr_read64(LOONGARCH_CSR_PGDL));
        switch (flag)
        {
        case UACCESS_R:
            if(is_page_dir(ptable_entry))
                permission = ptable_entry;
            else        
                permission = ptable_entry & (_PAGE_USER | __READABLE | _PAGE_PRESENT);
            break;
        case UACCESS_W:
            if(is_page_dir(ptable_entry))
                permission = ptable_entry;
            else        
                permission = ptable_entry & (_PAGE_USER | __WRITEABLE | _PAGE_PRESENT);
            break;
        case UACCESS_RW:
            permission = ptable_entry & (_PAGE_USER | __WRITEABLE | _PAGE_PRESENT | __READABLE);
            break;
        default:
            return false;
        }if (!permission)
        {
            return false;
        }mmu_check_start += PAGE_SIZE;
        mmu_check_size -= PAGE_SIZE;
    }return true;
}/**
 * @brief
 *    内核空间访问权限检查
 * @param[in] user_addr 用户态地址
 * @param[in] n 要检查的字节数
 * @param[in] flag 要检查的权限
 * @retval
 *   true 检查成功
 *   false 检查失败
 */
bool kernel_access_check (const void *kernel_addr, unsigned long n, int flag)
{
    unsigned long mmu_check_start;
    unsigned long mmu_check_size;
    uint64_t     ptable_entry = 0;
    uint64_t     permission;
    if (!kernel_addr || !n)
    {
        return false;
    }mmu_check_start = PAGE_SIZE_ALIGN ((unsigned long)kernel_addr);
    mmu_check_size  = roundup (n, PAGE_SIZE);
    while (mmu_check_size > 0)
    {
        ptable_entry = search_pgtable(mmu_check_start,csr_read64(LOONGARCH_CSR_PGDH));
        switch (flag)
        {
        case UACCESS_R:
            if(is_page_dir(ptable_entry))
                permission = ptable_entry;
            else
                permission = (ptable_entry & __READABLE) && (ptable_entry & _PAGE_PRESENT);
            break;
        case UACCESS_W:
            if(is_page_dir(ptable_entry))
                permission = ptable_entry;
            else        
                permission = (ptable_entry & __WRITEABLE) && (ptable_entry & _PAGE_PRESENT);
            break;
        default:
            return false;
        }if (!permission)
        {
            return false;
        }mmu_check_start += PAGE_SIZE;
        mmu_check_size -= PAGE_SIZE;
    }return true;
}static uint64_t page_entry_get(phys_addr_t pte, int level, int idx)
{
    uint64_t value;
    uint64_t * table = (uint64_t *)fix_map_set(FIX_MAP_PTABLE(level), pte, MT_KERNEL_MEM);
    value = table[idx];
    return value;
}static uint64_t search_pgtable(virt_addr_t v_addr,phys_addr_t mmu_base)
{
    phys_addr_t pte = mmu_base;
    uint64_t value;
    int       level;
    int       off;
    for (level = 0; level < MAX_TABLE_LEVEL + 1; level++)
    {
        off = GET_TABLE_OFF (level, v_addr);
        value = page_entry_get(pte, level, off);
        if (value == (uint64_t)NULL)
        {
            /* entry无效 */
            return 0ULL;
        }if((!(value & _PAGE_HUGE)) && (level < MAX_TABLE_LEVEL))
        {
            /* 如果不是最后一级，则取出表项，继续查找下一级 */
            pte = value & ENTRY_ADDRESS_MASK;
        }else
        {
            /* 最后一级页面，返回page */
            return value;
        }}
}static bool is_page_dir(uint64_t ptable_entry)
{
    if((ptable_entry != NULL) && (ptable_entry & 0xffful))
        return false;/* page */
    else
        return true;/* page dir */
}
