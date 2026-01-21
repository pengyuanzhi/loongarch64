/************************************************************************
 *				北京科银京成技术有限公司 版权所有
 * 	 Copyright (C) 2011 CoreTek Systems Inc. All Rights Reserved.
 ***********************************************************************/
/*
 * 修改历史：
 * 2017-04-12    李  燕，北京科银京成技术有限公司
 *               创建该文件。
 */
/*
 * @file： ttosObjALib.S 
 * @brief：
 *      <li>提供检查对象有效性相关的接口。</li>
 * @implements：
 */
/************************头 文 件******************************/
#define ASM_USE
#include <cpu.h>
#include <asm.h>
#include <ttosHandle.h>
/************************宏 定 义******************************/
/************************实    现******************************/
.global ttosVerifyObjectCore
.global ttosVerifyObjectCoreMagicRead;
.global ttosVerifyObjectCoreTypeRead;
.global ttosVerifyObjectCoreError;
/*
* @brief
*       判断对象的handle是否是有效的。
* @param[in]  objCore: 指向对象的objCore
* @param[in]  type:     需要与对象objCore中记录的对象类型进行 比较的类型
* @return
*       TTOS_VERIFY_OBJCORE_OK: 无效的objCore。
*       TTOS_VERIFY_OBJCORE_FAIL: 有效的objCore。
* @implements
*/
/*
T_UWORD ttosVerifyObjectCore(
T_TTOS_ObjectCoreID  objectCoreID,	
T_UWORD          type
)
*/
ENTRY(ttosVerifyObjectCore)
    BACKUP_T0T1
    /* a0=objectCoreID a1=type */
    beqz a0,ttosVerifyObjectCoreError
ttosVerifyObjectCoreMagicRead:
    ld.d    t0,a0,HANDLE_MAGIC  /* t0 = objectCoreID->handleId.magic */
    bne     a0,t0,ttosVerifyObjectCoreError /* objectCoreID == objectCoreID->handleId.magic? */
ttosVerifyObjectCoreTypeRead:
    ld.d t0,a0,HANDLE_TYPE  /* t0 = objectCoreID->handleId.type */
    bne  a1,t0,ttosVerifyObjectCoreError    /* no -> return TTOS_VERIFY_OBJCORE_FAIL */
    li.d a0,TTOS_VERIFY_OBJCORE_OK
    RELOAD_T0T1
    jirl  zero,ra,0
ttosVerifyObjectCoreError:
    li.d a0,TTOS_VERIFY_OBJCORE_FAIL
    RELOAD_T0T1
    jirl  zero,ra,0
ENDPROC(ttosVerifyObjectCore)
