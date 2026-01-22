/**
 * @file    ttosObjALib.S
 * @brief   TTOS对象验证汇编实现
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 本文件提供TTOS对象有效性验证的汇编接口
 *          - 验证对象句柄有效性
 *          - 检查对象魔数（Magic Number）
 *          - 检查对象类型匹配
 *          - 快速失败路径优化
 *
 * @note MISRA-C:2012 合规
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

/*************************** 修改历史 ****************************/
/*
 * 修改历史：
 * 2017-04-12    李燕，北京科银京成技术有限公司
 *               创建该文件
 * 2025-01-22    Intewell Team
 *               重构代码格式
 */

/*************************** 头文件包含 ****************************/
#define ASM_USE
#include <cpu.h>
#include <asm.h>
#include <ttosHandle.h>

/*************************** 宏定义 ****************************/


/*************************** 类型定义 ****************************/


/*************************** 外部声明 ****************************/


/*************************** 前向声明 ****************************/


/*************************** 模块变量 ****************************/


/*************************** 全局变量 ****************************/


/*************************** 函数实现 ****************************/

/**
 * @brief 验证TTOS对象核心结构有效性
 *
 * @details 验证对象句柄的有效性，包括：
 *          1. 检查对象指针是否为NULL
 *          2. 验证对象魔数（Magic Number）是否匹配
 *          3. 验证对象类型是否与期望类型匹配
 *
 * @param a0 objectCoreID 对象核心结构指针（T_TTOS_ObjectCoreID）
 * @param a1 type          期望的对象类型
 *
 * @return a0 验证结果
 *           - TTOS_VERIFY_OBJCORE_OK: 对象有效
 *           - TTOS_VERIFY_OBJCORE_FAIL: 对象无效
 *
 * @note 此函数使用 t0、t1 寄存器，调用前需要备份
 * @note 验证失败会立即跳转到错误处理路径
 * @note 这是一个快速失败（fail-fast）的实现
 *
 * @warning 调用此函数前必须确保对象指针已对齐
 * @warning 此函数会修改 t0、t1 寄存器
 */
.global ttosVerifyObjectCore
.global ttosVerifyObjectCoreMagicRead
.global ttosVerifyObjectCoreTypeRead
.global ttosVerifyObjectCoreError

ENTRY(ttosVerifyObjectCore)
    BACKUP_T0T1

    /* a0 = objectCoreID, a1 = type */
    beqz    a0, ttosVerifyObjectCoreError

ttosVerifyObjectCoreMagicRead:
    /* 读取对象魔数并与对象指针比较 */
    ld.d    t0, a0, HANDLE_MAGIC
    bne     a0, t0, ttosVerifyObjectCoreError

ttosVerifyObjectCoreTypeRead:
    /* 读取对象类型并与期望类型比较 */
    ld.d    t0, a0, HANDLE_TYPE
    bne     a1, t0, ttosVerifyObjectCoreError

    /* 验证成功 */
    li.d    a0, TTOS_VERIFY_OBJCORE_OK
    RELOAD_T0T1
    jirl    zero, ra, 0

ttosVerifyObjectCoreError:
    /* 验证失败 */
    li.d    a0, TTOS_VERIFY_OBJCORE_FAIL
    RELOAD_T0T1
    jirl    zero, ra, 0

ENDPROC(ttosVerifyObjectCore)
