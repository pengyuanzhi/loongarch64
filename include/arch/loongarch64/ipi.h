/**
 * @file    ipi.h
 * @brief   LoongArch64处理器间中断接口
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件定义LoongArch64处理器间中断(IPI)相关接口
 *          - IPI类型定义
 *          - IPI发送接口
 *          - IPI处理函数
 *
 * @note MISRA-C:2012 合规
 * @note IPI用于SMP多核处理器之间的通信
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef _IPI_H
#define _IPI_H

/************************头 文 件******************************/
#include <stdbool.h>
#include <system/types.h>
#include <driver/lsnIntCtlr.h>

/************************宏 定 义******************************/

/**
 * @defgroup IPIType IPI类型定义
 * @{
 */

/**
 * @brief 通用IPI调度中断
 */
#define GENERAL_IPI_SCHED    (CPU_IPI0 + 1U)

/**
 * @brief 通用IPI调用中断
 */
#define GENERAL_IPI_CALL     (CPU_IPI0 + 2U)

/** @} */

/************************类型定义******************************/

/**
 * @brief CPU集合类型（不完整声明）
 */
typedef struct cpu_set_t cpu_set_t;

/************************外部变量******************************/
/************************外部函数******************************/

/**
 * @defgroup IPIFunctions IPI功能函数
 * @{
 */

/**
 * @brief 发送重新调度IPI
 *
 * @details 向指定CPU集合发送重新调度IPI中断
 *
 * @param cpus         目标CPU集合
 * @param selfexcluded 是否排除当前CPU
 *
 * @return 成功返回0，失败返回错误码
 */
s32 ipi_reschedule(cpu_set_t *cpus, bool selfexcluded);

/**
 * @brief 重新调度IPI中断处理函数
 *
 * @details 处理重新调度IPI中断
 *
 * @param irq  中断号
 * @param param 参数
 *
 * @return 无
 */
void ipi_reschedule_handler(u32 irq, void *param);

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

#endif /* _IPI_H */
