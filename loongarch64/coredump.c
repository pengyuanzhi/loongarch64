/**
 * @file    coredump.c
 * @brief   LoongArch64核心转储功能
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件实现核心转储功能
 *          - 保存进程异常时的寄存器信息
 *          - 生成ELF格式的核心转储文件
 *
 * @note MISRA-C:2012 合规
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */
/************************头 文 件******************************/
#include <coredump.h>
#include <ttosProcess.h>
/************************函数实现******************************/
/**
 * @brief 保存进程寄存器信息到核心转储
 *
 * @details 保存进程发生用户态异常时的寄存器信息
 *          从异常上下文获取寄存器状态
 *
 * @param status 保存的目标位置
 * @param pcb    目标进程控制块
 *
 * @return 无
 *
 * @note 当前为占位实现
 */
void elf_save_regs(elf_prstatus_t *status, pcb_t pcb)
{
    printk("not support now !!! \n\r");
}
