/**
 * @file    symtab.h
 * @brief   符号表接口
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 提供符号表查询接口，用于栈回溯时显示函数名
 *          - 符号表项结构定义
 *          - 符号导出宏
 *          - 符号查找函数
 *
 * @note MISRA-C:2012 合规
 * @note 符号表用于内核调试和栈回溯
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef SYMTAB_H
#define SYMTAB_H

/*************************** 头文件包含 ****************************/
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*************************** 数据结构 ****************************/
/**
 * @brief 符号表项结构
 *
 * @details 包含符号名称和对应的地址值
 */
struct symtab_item
{
    const char *sym_name;  /**< @brief 符号名称字符串指针 */
    const void *sym_value; /**< @brief 符号对应的地址值 */
};

/*************************** 宏定义 ****************************/
/**
 * @defgroup SymbolExport 符号导出宏
 * @{
 */

#if defined(CONFIG_ALLSYMS)
/**
 * @brief 导出内核符号
 *
 * @details 将符号导出到全局符号表，使其可被查找
 *
 * @param symbol 要导出的符号名称
 */
#define KSYM_EXPORT(symbol) __attribute__((used, section(".ksym_export"))) typeof(symbol) symbol

/**
 * @brief 导出内核符号别名
 *
 * @details 为已有符号创建别名并导出到符号表
 *
 * @param old 原符号名称
 * @param symbol 新符号名称
 */
#define KSYM_EXPORT_ALIAS(old, symbol)                              \
    __attribute__((used, section(".ksym_export"))) typeof(old) old; \
    typeof(old) symbol __attribute__((__alias__(#old)))

#else
/**
 * @brief 空定义（未启用符号导出时）
 */
#define KSYM_EXPORT(...)
#endif

/** @} */

/*************************** 全局变量声明 ****************************/
/**
 * @brief 全局符号表数组
 *
 * @details 包含所有导出的内核符号
 */
extern struct symtab_item g_allsyms[];

/**
 * @brief 符号表项数量
 *
 * @details 全局符号表中符号的总数量
 */
extern int g_nallsyms;

/*************************** 函数声明 ****************************/
/**
 * @brief 根据名称查找符号
 *
 * @details 在指定的符号表中查找指定名称的符号
 *
 * @param symtab 符号表指针
 * @param name   符号名称
 * @param nsyms  符号表项数量
 *
 * @return 找到的符号表项指针，未找到返回NULL
 */
extern const struct symtab_item *symtab_findbyname(const struct symtab_item *symtab, const char *name, int nsyms);

/**
 * @brief 根据地址查找符号
 *
 * @details 在指定的符号表中查找包含指定地址的符号
 *
 * @param symtab 符号表指针
 * @param value  符号地址
 * @param nsyms  符号表项数量
 *
 * @return 找到的符号表项指针，未找到返回NULL
 */
extern const struct symtab_item *symtab_findbyvalue(const struct symtab_item *symtab, void *value, int nsyms);

/**
 * @brief 从全局符号表中根据名称查找符号
 *
 * @details 在全局符号表中查找指定名称的符号
 *
 * @param name  符号名称
 * @param size  输出参数，返回符号大小
 *
 * @return 找到的符号表项指针，未找到返回NULL
 */
extern const struct symtab_item *allsyms_findbyname(const char *name, size_t *size);

/**
 * @brief 从全局符号表中根据地址查找符号
 *
 * @details 查找包含指定地址的符号（用于栈回溯显示函数名）
 *
 * @param value 符号地址
 * @param size  输出参数，返回符号大小
 *
 * @return 找到的符号表项指针，未找到返回NULL
 *
 * @note 此函数常用于栈回溯时查找函数名
 */
extern const struct symtab_item *allsyms_findbyvalue(void *value, size_t *size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SYMTAB_H */
