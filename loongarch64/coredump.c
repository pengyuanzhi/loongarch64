/**********************************************************************************************
 *				科东(广州)软件科技有限公司 版权所有
 * 	 Copyright (C) 2021 Intewell (Guangzhou) Software Technology Co., Ltd.
 *All Rights Reserved.
 **********************************************************************************************/
/*
 * @file： arch/aarch64/coredump.h
 * @brief：
 *	    <li> coredump实现过程中与aarch64架构有关的部分  </li>
 */
#ifndef __LIBELF_COREDUMP_H__
#define __LIBELF_COREDUMP_H__
#include <coredump.h>
#include <ttosProcess.h>
/**
 * @brief
 *    保存进程发生用户态异常时的寄存器信息，从异常上下文获取
 * @param[in] status 保存的目标位置
 * @param[in] pcb  目标进程
 *
 */
void elf_save_regs (elf_prstatus_t *status, pcb_t pcb)
{
    printk("not support now !!! \n\r");
}
#endif
