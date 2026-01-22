/**
 * @file    ipi.c
 * @brief   LoongArch64核间中断(IPI)实现
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件实现LoongArch64核间中断功能
 *          - IPI发送和接收
 *          - 重调度IPI
 *          - 多核CPU集合管理
 *
 * @note MISRA-C:2012 合规
 * @note TODO: LOONGARCH实现细节待完善
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

/*************************** 头文件包含 ****************************/
#include <barrier.h>
#include <errno.h>
#include <ipi.h>
#include <system/bitops.h>
#include <system/types.h>
#include <ttos.h>
#include <ttosBase.h>
#include <ttos_pic.h>

/*************************** 宏定义 ****************************/

/*************************** 类型定义 ****************************/

/*************************** 外部声明 ****************************/
extern int32_t loongson2k_pic_ipi_ack(struct ttos_pic *pic, uint32_t *src_cpu, uint32_t *irq);

/*************************** 前向声明 ****************************/

/*************************** 模块变量 ****************************/

/*************************** 全局变量 ****************************/

/*************************** 函数实现 ****************************/

/**
 * @brief 发送IPI
 *
 * @details 向指定的CPU集合发送核间中断
 *
 * @param[in] cpus 目的CPU集合。为0时，表示系统中已经使能的CPU
 * @param[in] ipi ipi中断号
 * @param[in] selfexcluded 发送IPI的目的CPU集合是否排除自己
 *
 * @return 成功返回0，失败返回-EIO
 *
 * @retval 0 成功
 * @retval -EIO 失败
 */
static s32 ipi_send(cpu_set_t *cpus, u32 ipi, bool selfexcluded)
{
    cpu_set_t target_cpus;

    CPU_ZERO(&target_cpus);

    /* 获取目的cpu集合 */
    if (CPU_COUNT(cpus) == 0)
    {
#if CONFIG_SMP == 1
        CPU_OR(&target_cpus, &target_cpus, TTOS_CPUSET_ENABLED());
#else
        CPU_ZERO(&target_cpus);
        CPU_SET(0, &target_cpus);
#endif
    }
    else
    {
        CPU_OR(&target_cpus, &target_cpus, cpus);
    }

    /* 是否排除自己 */
    if (TRUE == selfexcluded)
    {
        CPU_CLR(cpuid_get(), &target_cpus);
    }

    if (CPU_COUNT(&target_cpus) == 0)
    {
        return (-EIO);
    }

    /* 获取有效位的cpu索引号 */
    for (int cpu = 0; cpu < CONFIG_MAX_CPUS; cpu++)
    {
        if (CPU_ISSET(cpu, &target_cpus))
        {
            if ((GENERAL_IPI_SCHED == ipi) && (FALSE == ttosIsNeedRescheduleWithCpuId(cpu)))
            {
                continue;
            }
            /* 发送ipi */
            ttos_pic_ipi_send(GENERAL_IPI_SCHED, cpu, 0);
        }
    }

    return (0);
}

/**
 * @brief 发送重调度IPI
 *
 * @details 向指定的CPU集合发送重调度核间中断
 *
 * @param[in] cpus 目的CPU集合。为0时，表示系统中已经使能的CPU
 * @param[in] selfexcluded 发送IPI的目的CPU集合是否排除自己
 *
 * @return 成功返回0，失败返回-EIO
 *
 * @retval 0 成功
 * @retval -EIO 失败
 */
s32 ipi_reschedule(cpu_set_t *cpus, bool selfexcluded)
{
    return ipi_send(cpus, GENERAL_IPI_SCHED, selfexcluded);
}

/**
 * @brief 重调度IPI处理函数
 *
 * @details 处理重调度IPI中断，对当前CPU上任务进行调度
 *
 * @param[in] irq 中断号
 * @param[in] param 私有数据
 *
 * @return 无
 *
 * @note 对于目前的调度策略，为空
 */
void ipi_reschedule_handler(u32 irq, void *param)
{
    ttosSchedule();    /* 对当前CPU上任务进行调度 */
}

/**
 * @brief LoongArch IPI中断处理函数
 *
 * @details 处理LoongArch核间中断
 *
 * @param[in] irq 中断号
 * @param[in] arg 参数
 *
 * @return 无
 */
void loongson_ipi_interrupt(uint32_t irq, void *arg)
{
    struct ttos_pic *pic_node;
    uint32_t from_cpu;

    pic_node = ttos_pic_get_pic(PIC_FLAG_CPU);
    if (pic_node != NULL)
    {
        while (loongson2k_pic_ipi_ack(pic_node, &from_cpu, &irq) == 0)
        {
            ttos_pic_irq_handle(irq);
            pic_node->pic_ops->pic_eoi(pic_node, irq, from_cpu);
        }
    }
}
