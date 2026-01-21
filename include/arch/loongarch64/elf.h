/**
 * @file    elf.h
 * @brief   LoongArch64 ELF格式定义
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件定义LoongArch64 ELF文件格式相关常量
 *          - ELF机器类型
 *          - 寄存器数量定义
 *          - 用于核心转储和调试
 *
 * @note MISRA-C:2012 合规
 * @note ELF是Executable and Linkable Format的缩写
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef __ARCH_LOONGARCH64_ELF_H
#define __ARCH_LOONGARCH64_ELF_H

/************************头 文 件******************************/
#include <stdint.h>
#include <sys/user.h>

/************************宏 定 义******************************/

/**
 * @defgroup ELFMachine ELF机器类型
 * @{
 */

/**
 * @brief LoongArch架构的ELF机器类型号
 *
 * @details 258是LoongArch在ELF规范中的机器类型标识
 */
#define EM_ARCH 258U

/** @} */

/**
 * @defgroup ELFRegisters ELF寄存器数量定义
 * @{
 */

/**
 * @brief 通用寄存器数量
 *
 * @details LoongArch通用寄存器包括：
 *          - GPRs: 32个通用寄存器
 *          - ORIG_A0: 原始参数寄存器
 *          - ERA: 异常返回地址
 *          - BADVADDR: 错误虚拟地址
 *          - CRMD: 当前模式寄存器
 *          - PRMD: 前一模式寄存器
 *          - EUEN: 扩展单元使能
 *          - ECFG: 异常配置寄存器
 *          - ESTAT: 异常状态寄存器
 *          - Reserved: 5个保留字段
 */
#define ELF_NGREG 45U

/**
 * @brief 浮点寄存器数量
 *
 * @details LoongArch浮点寄存器包括：
 *          - FPR: 32个浮点寄存器
 *          - FCC: 浮点条件码
 *          - FCSR: 浮点控制状态寄存器
 */
#define ELF_NFPREG 34U

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

#endif /* __ARCH_LOONGARCH64_INCLUDE_ELF_H */
