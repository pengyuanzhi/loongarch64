/**
 * @file    asm_module.h
 * @brief   汇编模块类型定义
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件定义汇编模块使用的64位类型
 *          - 根据配置定义64位有符号/无符号类型
 *          - LP64模式：long为64位
 *          - ILP32模式：long long为64位
 *
 * @note MISRA-C:2012 合规
 * @note 本文件主要用于汇编代码
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef __ASM_MODULE_H
#define __ASM_MODULE_H

/************************头 文 件******************************/
#include <limits.h>

/************************宏 定 义******************************/

/**
 * @brief LP64数据模型配置
 *
 * @details LP64: long和指针为64位
 *          ILP32: int、long和指针为32位
 */
#ifndef CONFIG_OS_LP64

/**
 * @brief ILP32模式64位类型定义
 */
typedef long long          s64; /**< 有符号64位整数 */
typedef unsigned long long u64; /**< 无符号64位整数 */

#else

/**
 * @brief LP64模式64位类型定义
 */
typedef long          s64; /**< 有符号64位整数 */
typedef unsigned long u64; /**< 无符号64位整数 */

#endif

/************************类型定义******************************/
/************************外部变量******************************/
/************************外部函数******************************/
/************************内联函数******************************/
/************************C++兼容性******************************/

#endif /* __ASM_MODULE_H */
