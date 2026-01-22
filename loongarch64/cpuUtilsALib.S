/**
 * @file    cpuUtilsALib.S
 * @brief   CPU工具函数汇编实现
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 本文件提供基本的CPU位运算相关的汇编接口
 *          - 获取前导零计数（32位）
 *          - 获取前导零计数（64位）
 *
 * @note MISRA-C:2012 合规
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

/*************************** 修改历史 ****************************/
/*
 * 修改历史：
 * 2021-02-24    彭元志，北京科银京成技术有限公司
 *               创建该文件
 * 2025-01-22    Intewell Team
 *               重构代码格式
 */

/*************************** 头文件包含 ****************************/
#define ASM_USE
#include <cpu.h>
#include <asm.h>

.file    "cpuUtilsALib.S"

/*************************** 宏定义 ****************************/


/*************************** 类型定义 ****************************/


/*************************** 外部声明 ****************************/


/*************************** 前向声明 ****************************/


/*************************** 模块变量 ****************************/


/*************************** 全局变量 ****************************/


/*************************** 函数实现 ****************************/

/**
 * @brief 获取32位整数的前导零数量
 *
 * @details 计算从最高位开始连续0的个数
 *          使用 clz.w 指令实现
 *
 * @param a0 numVar 32位无符号整数
 *
 * @return a0 前导零数量（0~32）
 *           - 如果输入为0，返回32
 *           - 如果输入为0x80000000，返回0
 *           - 如果输入为0xFFFFFFFF，返回0
 *
 * @note 从最高位（bit 31）向最低位（bit 0）计数
 * @note 这是 LoongArch64 的 clz（Count Leading Zeros）指令
 */
ENTRY(zeroNumOfWordGet)
    clz.w   a0, a0
    jirl    zero, ra, 0
ENDPROC(zeroNumOfWordGet)

/**
 * @brief 获取64位整数的前导零数量
 *
 * @details 计算从最高位开始连续0的个数
 *          使用 clz.d 指令实现
 *
 * @param a0 numVar 64位无符号整数
 *
 * @return a0 前导零数量（0~64）
 *           - 如果输入为0，返回64
 *           - 如果输入为0x8000000000000000，返回0
 *           - 如果输入为0xFFFFFFFFFFFFFFFF，返回0
 *
 * @note 从最高位（bit 63）向最低位（bit 0）计数
 * @note 这是 LoongArch64 的 clz（Count Leading Zeros）指令
 */
ENTRY(zeroNumOfLongGet)
    clz.d   a0, a0
    jirl    zero, ra, 0
ENDPROC(zeroNumOfLongGet)
