/**
 * @file    atomic.S
 * @brief   LoongArch64 原子操作汇编实现
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 本文件实现 LoongArch64 架构的原子操作
 *          - 32位原子加法（atomic32_add, atomic32_add_return）
 *          - 32位原子减法（atomic32_sub, atomic32_sub_return）
 *          - 32位原子自增/自减（atomic32_inc/dec, atomic32_inc/dec_return）
 *          - 32位原子位操作（atomic32Or, atomic32And, atomic32Xor）
 *          - 32位原子读写（atomic32_set, atomic32Get, atomic32Clear）
 *          - 32位原子比较交换（atomic32_cas）
 *
 * @note 使用 LoongArch64 的原子内存指令（amadd_db.w, amswap_db.w 等）
 * @note 所有操作都带有屏障语义（_db 后缀表示数据屏障）
 * @note 返回旧值的函数在修改前返回，返回新值的函数在修改后返回
 *
 * @note MISRA-C:2012 合规
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

/*************************** 修改历史 ****************************/
/*
 * 修改历史：
 * 2025-01-22    Intewell Team
 *               创建该文件
 */

/*************************** 头文件包含 ****************************/
#define ASM_USE
#include <asm.h>

/*************************** 宏定义 ****************************/


/*************************** 类型定义 ****************************/


/*************************** 外部声明 ****************************/


/*************************** 前向声明 ****************************/


/*************************** 模块变量 ****************************/


/*************************** 函数实现 ****************************/
.text

/**
 * @brief 32位原子加法（返回旧值）
 *
 * @details 将 target 指向的值增加 value，并返回 target 原有的值
 *          使用 amadd_db.w 指令实现原子加法
 *
 * @param a0 target   需要增加值的地址
 * @param a1 value   需要增加的值
 *
 * @return a0 target 原有指向的值
 */
ENTRY(atomic32_add)
	amadd_db.w t0, a1, a0
	move a0, t0
	jirl zero, ra, 0
	nop
ENDPROC(atomic32_add)

/**
 * @brief 32位原子加法（返回新值）
 *
 * @details 将 target 指向的值增加 value，并返回 target 增加后的值
 *          使用 amadd_db.w 指令实现原子加法
 *
 * @param a0 target   需要增加值的地址
 * @param a1 value   需要增加的值
 *
 * @return a0 target 增加后的值
 */
ENTRY(atomic32_add_return)
	amadd_db.w t0, a1, a0
	add.w t0, t0, a1
	move a0, t0
	jirl zero, ra, 0
	nop
ENDPROC(atomic32_add_return)

/**
 * @brief 32位原子按位或（返回旧值）
 *
 * @details 将 target 指向的值与 value 进行按位或操作，并返回 target 原有的值
 *          使用 amor_db.w 指令实现原子按位或
 *
 * @param a0 target   需要按位或的地址
 * @param a1 value   需要按位或的值
 *
 * @return a0 target 原有指向的值
 */
ENTRY(atomic32Or)
	amor_db.w t0, a1, a0
	move a0, t0
	jirl zero, ra, 0
	nop
ENDPROC(atomic32Or)

/**
 * @brief 32位原子按位与（返回旧值）
 *
 * @details 将 target 指向的值与 value 进行按位与操作，并返回 target 原有的值
 *          使用 amand_db.w 指令实现原子按位与
 *
 * @param a0 target   需要按位与的地址
 * @param a1 value   需要按位与的值
 *
 * @return a0 target 原有指向的值
 */
ENTRY(atomic32And)
	amand_db.w t0, a1, a0
	move a0, t0
	jirl zero, ra, 0
	nop
ENDPROC(atomic32And)

/**
 * @brief 32位原子设置（返回旧值）
 *
 * @details 设置 target 指向的值为 value，并返回 target 原有的值
 *          使用 amswap_db.w 指令实现原子交换
 *
 * @param a0 target   需要设置值的地址
 * @param a1 value   需要设置的值
 *
 * @return a0 target 原有指向的值
 */
ENTRY(atomic32_set)
	amswap_db.w t0, a1, a0
	move a0, t0
	jirl zero, ra, 0
	nop
ENDPROC(atomic32_set)

/**
 * @brief 32位原子读取
 *
 * @details 获取 target 指向的值
 *          使用 amadd_db.w 指令加 0 实现原子读取
 *
 * @param a0 target   需要获取值的地址
 *
 * @return a0 target 指向的值
 */
ENTRY(atomic32Get)
	amadd_db.w t0, zero, a0
	move a0, t0
	jirl zero, ra, 0
	nop
ENDPROC(atomic32Get)

/**
 * @brief 32位原子自增（返回旧值）
 *
 * @details 将 target 指向的值增加 1，并返回 target 原有的值
 *          使用 amadd_db.w 指令实现原子自增
 *
 * @param a0 target   需要增加值的地址
 *
 * @return a0 target 原有指向的值
 */
ENTRY(atomic32_inc)
	li.w t1, 1
	amadd_db.w t0, t1, a0
	move a0, t0
	jirl zero, ra, 0
	nop
ENDPROC(atomic32_inc)

/**
 * @brief 32位原子自增（返回新值）
 *
 * @details 将 target 指向的值增加 1，并返回 target 增加后的值
 *          使用 amadd_db.w 指令实现原子自增
 *
 * @param a0 target   需要增加值的地址
 *
 * @return a0 target 增加后的值
 */
ENTRY(atomic32_inc_return)
	li.w t1, 1
	amadd_db.w t0, t1, a0
	add.w t0, t0, t1
	move a0, t0
	jirl zero, ra, 0
	nop
ENDPROC(atomic32_inc_return)

/**
 * @brief 32位原子自减（返回旧值）
 *
 * @details 将 target 指向的值减去 1，并返回 target 原有的值
 *          使用 amadd_db.w 指令加 -1 实现原子自减
 *
 * @param a0 target   需要减去值的地址
 *
 * @return a0 target 原有指向的值
 */
ENTRY(atomic32_dec)
	li.w t1, -1
	amadd_db.w t0, t1, a0
	move a0, t0
	jirl zero, ra, 0
	nop
ENDPROC(atomic32_dec)

/**
 * @brief 32位原子自减（返回新值）
 *
 * @details 将 target 指向的值减去 1，并返回 target 减去后的值
 *          使用 amadd_db.w 指令加 -1 实现原子自减
 *
 * @param a0 target   需要减去值的地址
 *
 * @return a0 target 减去后的值
 */
ENTRY(atomic32_dec_return)
	li.w t1, -1
	amadd_db.w t0, t1, a0
	add.w t0, t0, t1
	move a0, t0
	jirl zero, ra, 0
	nop
ENDPROC(atomic32_dec_return)

/**
 * @brief 32位原子比较并交换（CAS）
 *
 * @details 将 target 指向的值与 oldValue 比较
 *          如果相等，设置 target 指向的值为 newValue，并返回 TRUE
 *          如果不相等，则直接返回 FALSE
 *          使用 LL/SC（Load-Linked/Store-Conditional）指令对实现
 *
 * @param a0 target    需要比较值的地址
 * @param a1 oldValue  需要比较的值
 * @param a2 newValue  需要设置的值
 *
 * @return a0 TRUE（1）: target 指向的值与 oldValue 相等
 * @return a0 FALSE（0）: target 指向的值与 oldValue 不相等
 */
ENTRY(atomic32_cas)
	li.w t1, 0          /* 设置初始返回状态为 FALSE */
1:
	dbar 0              /* 数据屏障 */
	ll.w t0, a0, 0      /* 获取当前值并建立原子检查 */
	bne t0, a1, 2f      /* 如果不相等，跳转到标签 2 */
	nop
	move t2, a2
	sc.w t2, a0, 0      /* 条件存储 */
	beqz t2, 1b         /* 如果原子操作未成功，重试 */
	nop
	li.w t1, 1          /* 存储成功，返回 TRUE */
2:
	dbar 0              /* 数据屏障 */
	move a0, t1
	jirl zero, ra, 0
	nop
ENDPROC(atomic32_cas)

/**
 * @brief 32位原子减法（返回旧值）
 *
 * @details 将 target 指向的值减去 value，并返回 target 原有的值
 *          使用 LL/SC 指令对实现原子减法
 *
 * @param a0 target   需要减去值的地址
 * @param a1 value   需要减去的值
 *
 * @return a0 target 原有指向的值
 */
ENTRY(atomic32_sub)
1:
	dbar 0              /* 数据屏障 */
	ll.w t1, a0, 0      /* 获取当前值并建立原子检查 */
	sub.w t0, t1, a1
	sc.w t0, a0, 0
	beqz t0, 1b         /* 如果原子操作未成功，重试 */
	move a0, t1
	jirl zero, ra, 0
	nop
ENDPROC(atomic32_sub)

/**
 * @brief 32位原子减法（返回新值）
 *
 * @details 将 target 指向的值减去 value，并返回 target 减去后的值
 *          使用 LL/SC 指令对实现原子减法
 *
 * @param a0 target   需要减去值的地址
 * @param a1 value   需要减去的值
 *
 * @return a0 target 减去后的值
 */
ENTRY(atomic32_sub_return)
1:
	dbar 0              /* 数据屏障 */
	ll.w t1, a0, 0      /* 获取当前值并建立原子检查 */
	sub.w t0, t1, a1
	sc.w t0, a0, 0
	beqz t0, 1b         /* 如果原子操作未成功，重试 */
	move a0, t0
	jirl zero, ra, 0
	nop
ENDPROC(atomic32_sub_return)

/**
 * @brief 32位原子清除（返回旧值）
 *
 * @details 清除 target 指向的值（设置为 0），并返回 target 原有的值
 *          使用 amswap_db.w 指令实现原子清除
 *
 * @param a0 target   需要清除值的地址
 *
 * @return a0 target 原有指向的值
 */
ENTRY(atomic32Clear)
	amswap_db.w t0, zero, a0
	move a0, t0
	jirl zero, ra, 0
	nop
ENDPROC(atomic32Clear)

/**
 * @brief 32位原子按位异或（返回旧值）
 *
 * @details 将 target 指向的值与 value 进行按位异或操作，并返回 target 原有的值
 *          使用 amxor_db.w 指令实现原子按位异或
 *
 * @param a0 target   需要按位异或的地址
 * @param a1 value   需要按位异或的值
 *
 * @return a0 target 原有指向的值
 */
ENTRY(atomic32Xor)
	amxor_db.w t0, a1, a0
	move a0, t0
	jirl zero, ra, 0
	nop
ENDPROC(atomic32Xor)
