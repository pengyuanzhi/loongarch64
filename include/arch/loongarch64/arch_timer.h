/**
 * @file    arch_timer.h
 * @brief   LoongArch64架构时钟接口
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件定义LoongArch64架构时钟相关接口
 *          - 常量时钟频率计算
 *          - 稳定时钟配置
 *          - 时钟频率读取接口
 *
 * @note MISRA-C:2012 合规
 * @note 使用LoongArch CPUCFG寄存器读取时钟信息
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef _ARCH_TIMER_H
#define _ARCH_TIMER_H

/************************头 文 件******************************/
#include <cpu.h>

/************************宏 定 义******************************/

/**
 * @brief 系统时钟频率（Hz）
 *
 * @details 定义系统时钟节拍频率为1000Hz
 *          每毫秒产生一次时钟中断
 */
#undef HZ
#define HZ 1000U

/************************类型定义******************************/
/************************外部变量******************************/
/************************外部函数******************************/

/************************内联函数******************************/

/**
 * @brief 计算常量时钟频率
 *
 * @details 从LoongArch CPUCFG寄存器读取时钟配置信息
 *          计算稳定时钟频率（LLFTP）
 *
 * @return 成功返回时钟频率（Hz），失败返回0
 *
 * @note 时钟频率 = base_freq * cfm / cfd
 * @note 需要硬件支持LLFTP特性
 */
static inline u32 calc_const_freq(void)
{
    u32 res;
    u32 base_freq;
    u32 cfm;
    u32 cfd;

    /* 读取CPUCFG2，检查LLFTP支持 */
    res = read_cpucfg(LOONGARCH_CPUCFG2);
    if (!(res & CPUCFG2_LLFTP))
    {
        return 0U;
    }

    /* 读取基础频率 */
    base_freq = read_cpucfg(LOONGARCH_CPUCFG4);

    /* 读取时钟倍频和分频因子 */
    res = read_cpucfg(LOONGARCH_CPUCFG5);
    cfm = res & 0xffffU;
    cfd = (res >> 16U) & 0xffffU;

    /* 参数有效性检查 */
    if ((base_freq == 0U) || (cfm == 0U) || (cfd == 0U))
    {
        return 0U;
    }

    /* 计算实际频率 */
    return (base_freq * cfm / cfd);
}

/************************C++兼容性******************************/
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif /* _ARCH_TIMER_H */
