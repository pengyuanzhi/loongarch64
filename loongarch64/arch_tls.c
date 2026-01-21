/**
 * @file    arch_tls.c
 * @brief   LoongArch64线程本地存储（TLS）管理
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件实现线程本地存储（Thread Local Storage）功能
 *          - 设置TLS指针
 *          - 获取TLS指针
 *          - 使用寄存器r2（tp寄存器）快速访问
 *
 * @note MISRA-C:2012 合规
 * @note LoongArch使用r2寄存器作为线程指针（tp）
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */
/*************************** 头文件包含 ****************************/
#include <stdint.h>
/*************************** 全局变量 ****************************/
/**
 * @brief TLS指针寄存器变量
 *
 * @details 使用r2寄存器（tp，线程指针）存储TLS地址
 *          使用register关键字确保变量始终映射到硬件寄存器
 *
 * @note r2是LoongArch架构专门用于线程指针的寄存器
 */
register unsigned long __my_tls __asm__("$r2");
/*************************** 函数实现 ****************************/
/**
 * @brief 设置线程本地存储指针
 *
 * @details 将TLS指针设置为指定的值
 *          同时更新硬件寄存器r2（tp）
 *
 * @param tls 要设置的TLS指针值
 *
 * @return 无
 *
 * @note 此函数通常在线程创建时调用
 */
void kernel_set_tls(uintptr_t tls)
{
    __my_tls = tls;
}

/**
 * @brief 获取线程本地存储指针
 *
 * @details 返回当前线程的TLS指针
 *          直接从r2寄存器（tp）读取
 *
 * @param 无
 *
 * @return 当前TLS指针值
 *
 * @note 此函数非常高效，因为只读取一个寄存器
 */
uintptr_t kernel_get_tls(void)
{
    return __my_tls;
}
