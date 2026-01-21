/**
 * @file    cpu-features.h
 * @brief   LoongArch64 CPU特性检测接口
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件定义LoongArch64 CPU特性检测宏
 *          - CPU特性位检测宏
 *          - ISA级别检测
 *          - 各种CPU扩展特性检测
 *          - 地址位数配置
 *
 * @note MISRA-C:2012 合规
 * @note 假设CPU0的特性是所有CPU的超集（SMP）
 *
 * @copyright Copyright (c) 2025 Intewell Team
 * @copyright Copyright (C) 2020-2022 Loongson Technology Corporation Limited
 */

#ifndef _CPU_FEATURES_H
#define _CPU_FEATURES_H

/************************头 文 件******************************/
#include <cpu.h>
#include <cpu-info.h>

/************************宏 定 义******************************/

/**
 * @defgroup CPUFeatureDetection CPU特性检测基础宏
 * @{
 */

/**
 * @brief 检测CPU选项位
 *
 * @details 检查CPU0的options字段中指定选项位是否置位
 */
#define cpu_opt(opt)            (cpu_data[0U].options & (opt))

/**
 * @brief 检测CPU特性
 *
 * @details 检查CPU0是否支持指定特性（使用位索引）
 */
#define cpu_has(feat)            (cpu_data[0U].options & BIT_ULL(feat))

/** @} */

/**
 * @defgroup ISAISADetection ISA级别检测
 * @{
 */

/**
 * @brief 检测LoongArch架构
 */
#define cpu_has_loongarch        (cpu_has_loongarch32 | cpu_has_loongarch64)

/**
 * @brief 检测32位LoongArch ISA
 */
#define cpu_has_loongarch32      (cpu_data[0U].isa_level & LOONGARCH_CPU_ISA_32BIT)

/**
 * @brief 检测64位LoongArch ISA
 */
#define cpu_has_loongarch64      (cpu_data[0U].isa_level & LOONGARCH_CPU_ISA_64BIT)

/** @} */

/**
 * @defgroup AddressBits 地址位数配置
 * @{
 */

#ifdef CONFIG_32BIT

#define cpu_has_64bits            (cpu_data[0U].isa_level & LOONGARCH_CPU_ISA_64BIT)
#define cpu_vabits               31U /**< 虚拟地址位数（32位模式） */
#define cpu_pabits               31U /**< 物理地址位数（32位模式） */

#endif

#ifdef CONFIG_64BIT

#define cpu_has_64bits            1U  /**< 支持64位 */
#define cpu_vabits               cpu_data[0U].vabits /**< 虚拟地址位数 */
#define cpu_pabits               cpu_data[0U].pabits /**< 物理地址位数 */
#define __NEED_ADDRBITS_PROBE

#endif

/** @} */

/**
 * @defgroup CPUFeatures CPU特性检测宏
 * @brief 检测各种CPU扩展特性
 * @note SMP假设：CPU0的选项是所有处理器的超集（所有已知LoongArch系统成立）
 * @{
 */

#define cpu_has_cpucfg           cpu_opt(LOONGARCH_CPU_CPUCFG)      /**< CPUCFG寄存器 */
#define cpu_has_lam              cpu_opt(LOONGARCH_CPU_LAM)         /**< 原子指令 */
#define cpu_has_ual              cpu_opt(LOONGARCH_CPU_UAL)         /**< 非对齐访问 */
#define cpu_has_fpu              cpu_opt(LOONGARCH_CPU_FPU)         /**< 浮点单元 */
#define cpu_has_lsx              cpu_opt(LOONGARCH_CPU_LSX)         /**< 128位SIMD */
#define cpu_has_lasx             cpu_opt(LOONGARCH_CPU_LASX)        /**< 256位SIMD */
#define cpu_has_crc32            cpu_opt(LOONGARCH_CPU_CRC32)       /**< CRC32指令 */
#define cpu_has_complex          cpu_opt(LOONGARCH_CPU_COMPLEX)     /**< 复杂指令 */
#define cpu_has_crypto           cpu_opt(LOONGARCH_CPU_CRYPTO)      /**< 加密指令 */
#define cpu_has_lvz              cpu_opt(LOONGARCH_CPU_LVZ)         /**< 虚拟化扩展 */
#define cpu_has_lbt_x86          cpu_opt(LOONGARCH_CPU_LBT_X86)     /**< x86二进制翻译 */
#define cpu_has_lbt_arm          cpu_opt(LOONGARCH_CPU_LBT_ARM)     /**< ARM二进制翻译 */
#define cpu_has_lbt_mips         cpu_opt(LOONGARCH_CPU_LBT_MIPS)    /**< MIPS二进制翻译 */
#define cpu_has_lbt              (cpu_has_lbt_x86 | cpu_has_lbt_arm | cpu_has_lbt_mips) /**< 二进制翻译 */
#define cpu_has_csr              cpu_opt(LOONGARCH_CPU_CSR)         /**< CSR寄存器 */
#define cpu_has_iocsr            cpu_opt(LOONGARCH_CPU_IOCSR)       /**< IOCSR寄存器 */
#define cpu_has_tlb              cpu_opt(LOONGARCH_CPU_TLB)         /**< TLB */
#define cpu_has_watch            cpu_opt(LOONGARCH_CPU_WATCH)       /**< 观察点寄存器 */
#define cpu_has_vint             cpu_opt(LOONGARCH_CPU_VINT)        /**< 向量化中断 */
#define cpu_has_csripi           cpu_opt(LOONGARCH_CPU_CSRIPI)      /**< CSR-IPI */
#define cpu_has_extioi           cpu_opt(LOONGARCH_CPU_EXTIOI)      /**< 扩展IO中断 */
#define cpu_has_prefetch         cpu_opt(LOONGARCH_CPU_PREFETCH)    /**< 预取指令 */
#define cpu_has_pmp              cpu_opt(LOONGARCH_CPU_PMP)         /**< 性能监控计数器 */
#define cpu_has_perf             cpu_opt(LOONGARCH_CPU_PMP)         /**< 性能监控（别名） */
#define cpu_has_scalefreq        cpu_opt(LOONGARCH_CPU_SCALEFREQ)   /**< 频率缩放 */
#define cpu_has_flatmode         cpu_opt(LOONGARCH_CPU_FLATMODE)    /**< 平坦模式 */
#define cpu_has_eiodecode        cpu_opt(LOONGARCH_CPU_EIODECODE)   /**< EXTIOI中断引脚解码 */
#define cpu_has_guestid          cpu_opt(LOONGARCH_CPU_GUESTID)     /**< GuestID特性 */
#define cpu_has_hypervisor       cpu_opt(LOONGARCH_CPU_HYPERVISOR)  /**< 运行在虚拟机中 */
#define cpu_has_ptw              cpu_opt(LOONGARCH_CPU_PTW)         /**< 硬件页表遍历器 */
#define cpu_has_lspw             cpu_opt(LOONGARCH_CPU_LSPW)        /**< LSPW (lddir/ldpte指令) */
#define cpu_has_avecint          cpu_opt(LOONGARCH_CPU_AVECINT)     /**< AVEC中断 */

/** @} */

/************************类型定义******************************/
/************************外部变量******************************/
/************************外部函数******************************/
/************************内联函数******************************/
/************************C++兼容性******************************/

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif /* __ASM_CPU_FEATURES_H */
