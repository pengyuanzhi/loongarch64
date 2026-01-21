/**
 * @file    atomic.c
 * @brief   LoongArch64原子操作实现
 * @author  AISafe64 Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件实现了LoongArch64原子操作相关功能
 *          - 原子比较交换
 *          - 多核原子操作支持
 *
 * @note MISRA-C:2012 合规
 * @warning 原子操作在多核环境下必须正确使用内存屏障
 *
 * @copyright Copyright (c) 2025 AISafe64 Team
 */
/*************************** 头文件包含 ****************************/
#include <atomic.h>
/*************************** 函数实现 ****************************/
/**
 * @brief 32位原子比较交换
 *
 * @details 原子地比较指针指向的值与old值
 *          如果相等，则设置为new值
 *
 * @param ptr  目标指针（不能为NULL）
 * @param old  期望值的指针（不能为NULL）
 * @param new  新值
 *
 * @return 成功返回0，失败返回非0
 *
 * @note 此函数是对atomic32_cas的封装
 * @note 操作是原子的，适合多核环境
 */
int atomic32_compare_exchange(volatile int *ptr, int *old, int new)
{
    return atomic32_cas(ptr, *old, new);
}
