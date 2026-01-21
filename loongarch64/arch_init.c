/**
 * @file    arch_init.c
 * @brief   LoongArch64架构初始化
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件实现LoongArch64架构的初始化功能
 *          - 启动处理器（BSP）初始化
 *          - 应用处理器（AP）初始化
 *          - 早期MMU初始化
 *
 * @note MISRA-C:2012 合规
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */
/*************************** 头文件包含 ****************************/
#include <adrspace.h>
#include <cpu.h>
#include <exception.h>
#include <mmu.h>
#include <percpu.h>
#include <stdint.h>
#include <system/const.h>
#include <system/types.h>
#include <ttosMM.h>
/*************************** 外部变量声明 ****************************/

extern long long _start;    /**< 内核起始地址 */
extern int __image_start__; /**< 镜像起始地址 */
extern int __end__;         /**< 镜像结束地址 */

/*************************** 外部函数声明 ****************************/
/**
 * @brief 异常处理初始化
 */
extern void exception_init(void);
/**
 * @brief CPU探测和初始化
 */
extern void cpu_probe(void);
/**
 * @brief 断点异常初始化
 */
extern void bp_exception_init(void);

/**
 * @brief 应用处理器异常初始化
 */
extern void ap_exception_init(void);

/*************************** 内部函数声明 ****************************/
/**
 * @brief 设置内核MMU物理偏移
 *
 * @param pa_l 物理地址低32位
 * @param pa_h 物理地址高32位
 * @param va   虚拟地址
 */
void kernel_mmu_set_pvoffset(uint32_t pa_l, uint32_t pa_h, virt_addr_t va);
/**
 * @brief 陷阱初始化
 */
void trap_init(void);
/**
 * @brief 初始化应用处理器
 */
extern void loongson_init_secondary(void);

/*************************** 函数实现 ****************************/
/**
 * @brief 早期MMU初始化
 *
 * @details 在启动阶段设置内核虚拟地址到物理地址的映射
 *
 * @param 无
 *
 * @return 无
 *
 * @note 此函数必须在完整MMU初始化之前调用
 */
void early_mmu_init(void)
{
    kernel_mmu_set_pvoffset(PHYSADDR((&_start)), 0U, (virt_addr_t)(&_start));
}

/**
 * @brief 启动处理器（BSP）架构初始化
 *
 * @details 初始化LoongArch64启动处理器（Boot Processor）
 *          - 探测CPU特性
 *          - 初始化异常处理
 *          - 初始化MMU
 *          - 初始化陷阱向量
 *
 * @param 无
 *
 * @return 无
 *
 * @note 此函数仅在启动处理器（CPU0）上执行
 */
void bp_arch_init(void)
{
    cpu_probe();
    exception_init();
    early_mmu_init();
    trap_init();
}

/**
 * @brief 应用处理器（AP）架构初始化
 *
 * @details 初始化LoongArch64应用处理器（Application Processor）
 *          - 探测CPU特性
 *          - 初始化异常处理
 *          - 初始化辅助处理器特性
 *
 * @param 无
 *
 * @return 无
 *
 * @note 此函数在所有非启动处理器上执行
 */
void ap_arch_init(void)
{
    cpu_probe();
    exception_init();
    loongson_init_secondary();
}
