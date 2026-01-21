/**
 * @file    cpu.c
 * @brief   LoongArch64 CPU管理实现
 * @author  AISafe64 Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件实现了LoongArch64 CPU相关管理功能
 *          - CPU ID获取与设置
 *          - 启动CPU检测
 *          - 多核SMP初始化
 *          - 位操作辅助函数
 *
 * @note MISRA-C:2012 合规
 * @warning 多核操作需要正确使用内存屏障
 *
 * @copyright Copyright (c) 2025 AISafe64 Team
 */
/************************头 文 件******************************/
#include <cpu.h>
#include <percpu.h>
#include <limits.h>
#include <stdbool.h>
#include <driver/cpudev.h>
/************************宏 定 义******************************/
/************************类型定义******************************/
/************************全局变量******************************/
/* MAP BASE */
unsigned long vm_map_base;        /**< @brief 虚拟内存映射基地址 */
/* Virtual Address size in bits */
unsigned long g_vaBits;           /**< @brief 虚拟地址宽度（位） */
/* Physical Address size in bits */
unsigned long g_paBits;           /**< @brief 物理地址宽度（位） */
/************************外部声明******************************/
/************************前向声明******************************/
/************************模块变量******************************/
/************************函数实现******************************/
/**
 * @brief 设置CPU ID
 *
 * @details 设置当前CPU的ID标识
 *
 * @param 无
 *
 * @return 无
 *
 * @note 当前实现为空，保留用于未来扩展
 */
void cpuid_set(void)
{
    struct cpudev *cpu = cpu_self_get();
    /* TODO: 实现CPU ID设置逻辑 */
}
/**
 * @brief 获取CPU ID
 *
 * @details 从LOONGARCH_CSR_CPUID寄存器读取当前CPU的ID
 *
 * @param 无
 *
 * @return 返回当前CPU的ID
 *
 * @note 此函数通过CSR寄存器读取硬件CPU ID
 */
u32 cpuid_get(void)
{
    return (u32)csr_read32(LOONGARCH_CSR_CPUID);
}
/**
 * @brief 判断是否为启动CPU
 *
 * @details 检查当前CPU是否为系统启动时的主CPU（Boot CPU）
 *
 * @param 无
 *
 * @return true  当前是启动CPU
 * @return false 当前非启动CPU（辅助核）
 *
 * @note 启动CPU负责系统初始化，辅助核在启动后被激活
 */
bool is_bootcpu(void)
{
    struct cpudev *cpu = cpu_self_get();
    return cpu->boot_cpu;
}
/**
 * @brief 查找第一个零位
 *
 * @details 在给定的长整型中查找第一个为零的位的位置
 *
 * @param x 要搜索的长整型值
 *
 * @return 返回第一个零位的位置（0-based），如果全为1则返回-1
 *
 * @note 使用__builtin_ffsl内建函数实现
 * @note __builtin_ffsl返回第一个1的位置，因此需要先按位取反
 */
unsigned long ttos_ffz(unsigned long x)
{
    return (unsigned long)(__builtin_ffsl(~x) - 1U);
}
/**
 * @brief 初始化辅助CPU
 *
 * @details 在SMP系统中初始化辅助CPU（Secondary CPU）
 *          配置中断掩码和IPI（核间中断）使能
 *
 * @param 无
 *
 * @return 无
 *
 * @note 此函数仅在辅助CPU上执行
 * @note 配置了IP0/IP1/IP2、IPI、PMC、TIMER、SIP0中断
 * @note 使能所有IPI中断（0xffffffff）
 */
void loongson_init_secondary(void)
{
    unsigned int imask = ECFGF_IP0 | ECFGF_IP1 | ECFGF_IP2 |
                         ECFGF_IPI | ECFGF_PMC | ECFGF_TIMER | ECFGF_SIP0;
    change_csr_ecfg(ECFG0_IM, imask);
    iocsr_write32(0xffffffffU, LOONGARCH_IOCSR_IPI_EN);
}
