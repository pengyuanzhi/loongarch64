/**
 * @file    arch-uaccess.h
 * @brief   LoongArch64用户空间访问接口
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件定义用户空间访问相关宏和接口
 *          - __get_user: 从用户空间读取数据
 *          - __put_user: 向用户空间写入数据
 *          - 异常表集成，处理访问错误
 *
 * @note MISRA-C:2012合规
 * @note 用户空间访问可能触发异常，需要异常表支持
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef _ARCH_UACCESS_H
#define _ARCH_UACCESS_H

/* ==================== 头文件包含 ==================== */
#include <asm-extable.h>

/* ==================== 宏定义 ==================== */

/**
 * @brief 编译时断言宏（空实现）
 *
 * @details 用于编译时检查条件，当前为空实现
 */
#define BUILD_BUG_ON_MSG(cond, msg)

/**
 * @brief 大结构体包装宏
 *
 * @details 用于强制内联汇编使用指定大小的访问
 */
struct __large_struct
{
    unsigned long buf[100U];
};

#define __m(x) (*(struct __large_struct __user *)(x))

/**
 * @defgroup GetUser 从用户空间读取数据
 * @{
 */

/**
 * @brief 从用户空间读取简单变量
 *
 * @details 从用户空间地址读取数据到内核空间
 *          支持char、int等简单类型
 *
 * @param x   存储结果的变量
 * @param ptr 用户空间源地址
 *
 * @return 成功返回0，失败返回-EFAULT
 *
 * @note 调用者必须先用access_ok()检查指针
 * @note 失败时x被设置为0
 */
#define __get_user(x, ptr)                           \
    ({                                               \
        int __gu_err = 0;                            \
        __get_user_common((x), sizeof(*(ptr)), ptr); \
        __gu_err;                                    \
    })

/**
 * @brief 通用用户空间读取宏
 *
 * @details 根据数据大小选择相应的加载指令
 */
#define __get_user_common(val, size, ptr)            \
    do                                               \
    {                                                \
        switch (size)                                \
        {                                            \
        case 1U:                                     \
            __get_data_asm(val, "ld.b", ptr);        \
            break;                                   \
        case 2U:                                     \
            __get_data_asm(val, "ld.h", ptr);        \
            break;                                   \
        case 4U:                                     \
            __get_data_asm(val, "ld.w", ptr);        \
            break;                                   \
        case 8U:                                     \
            __get_data_asm(val, "ld.d", ptr);        \
            break;                                   \
        default:                                     \
            BUILD_BUG_ON_MSG(1, "BUILD_BUG failed"); \
            break;                                   \
        }                                            \
    } while (0)

/**
 * @brief 汇编指令读取用户空间数据
 *
 * @details 使用指定指令从用户空间读取数据
 *          包含异常表处理
 */
#define __get_data_asm(val, insn, ptr)                                                                  \
    {                                                                                                   \
        long __gu_tmp;                                                                                  \
                                                                                                        \
        __asm__ __volatile__("1:    " insn "    %1, %2            \n"                                   \
                             "2:                    \n" _ASM_EXTABLE_UACCESS_ERR_ZERO(1b, 2b, % 0, % 1) \
                             : "+r"(__gu_err), "=r"(__gu_tmp)                                           \
                             : "m"(__m(ptr)));                                                          \
                                                                                                        \
        (val) = (__typeof__(*(ptr)))__gu_tmp;                                                           \
    }

/** @} */

/**
 * @defgroup PutUser 向用户空间写入数据
 * @{
 */

/**
 * @brief 向用户空间写入简单值
 *
 * @details 从内核空间写入数据到用户空间地址
 *          支持char、int等简单类型
 *
 * @param x   要写入的值
 * @param ptr 用户空间目标地址
 *
 * @return 成功返回0，失败返回-EFAULT
 *
 * @note 调用者必须先用access_ok()检查指针
 */
#define __put_user(x, ptr)                      \
    ({                                          \
        int __pu_err = 0;                       \
        __typeof__(*(ptr)) __pu_val;            \
        __pu_val = (x);                         \
        __put_user_common(ptr, sizeof(*(ptr))); \
        __pu_err;                               \
    })

/**
 * @brief 通用用户空间写入宏
 *
 * @details 根据数据大小选择相应的存储指令
 */
#define __put_user_common(ptr, size)                 \
    do                                               \
    {                                                \
        switch (size)                                \
        {                                            \
        case 1U:                                     \
            __put_data_asm("st.b", ptr);             \
            break;                                   \
        case 2U:                                     \
            __put_data_asm("st.h", ptr);             \
            break;                                   \
        case 4U:                                     \
            __put_data_asm("st.w", ptr);             \
            break;                                   \
        case 8U:                                     \
            __put_data_asm("st.d", ptr);             \
            break;                                   \
        default:                                     \
            BUILD_BUG_ON_MSG(1, "BUILD_BUG failed"); \
            break;                                   \
        }                                            \
    } while (0)

/**
 * @brief 汇编指令写入用户空间数据
 *
 * @details 使用指定指令向用户空间写入数据
 *          包含异常表处理
 */
#define __put_data_asm(insn, ptr)                                                             \
    {                                                                                         \
        __asm__ __volatile__("1:    " insn "    %z2, %1        # __put_user_asm\n"            \
                             "2:                    \n" _ASM_EXTABLE_UACCESS_ERR(1b, 2b, % 0) \
                             : "+r"(__pu_err), "=m"(__m(ptr))                                 \
                             : "Jr"(__pu_val));                                               \
    }

/** @} */

/* ==================== 类型定义 ==================== */
/* 本文件不定义类型 */

/* ==================== C++兼容性 ==================== */
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif /* _ARCH_UACCESS_H */
