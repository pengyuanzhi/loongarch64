/**
 * @file    backtrace.h
 * @brief   LoongArch64栈回溯接口
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 提供LoongArch64架构的栈回溯功能，用于调试和异常处理
 *          - 显示调用栈
 *          - 显示函数名和偏移量
 *          - 支持从指定帧地址开始回溯
 *
 * @note MISRA-C:2012 合规
 * @warning 使用-fno-omit-frame-pointer编译选项以确保帧指针可用
 * @warning 此功能可能泄露敏感信息，仅用于调试构建
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef BACKTRACE_H
#define BACKTRACE_H

#include <stdint.h>

/**
 * @brief 显示栈回溯信息
 *
 * @details 从当前栈位置开始回溯，显示调用栈和函数名
 *          输出格式：BACKTRACE: START: <cookie>
 *                   <level>: caller: <address> (<function> + 0x<offset>)
 *                   BACKTRACE: END: <cookie>
 *
 * @param cookie 用于标识和过滤日志消息的字符串
 *
 * @note 需要-fno-omit-frame-pointer编译选项
 * @note 使用链表遍历帧记录，最大深度为UNWIND_LIMIT
 * @warning 如果栈损坏，此函数可能显示敏感信息
 *
 * @par 示例代码
 * @code
 * void some_function(void)
 * {
 *     if (error_detected)
 *     {
 *         backtrace("error_case");
 *     }
 * }
 * @endcode
 *
 * @see backtrace_r()
 */
void backtrace(const char *cookie);

/**
 * @brief 从指定帧地址开始回溯栈
 *
 * @details 从给定的帧地址开始回溯，用于异常处理等场景
 *          输出格式：BACKTRACE: START: <cookie>
 *                   <level>: caller: <address> (<function> + 0x<offset>)
 *                   BACKTRACE: END: <cookie>
 *
 * @param cookie         用于标识和过滤日志消息的字符串
 * @param frame_address  帧记录地址
 *
 * @note frame_address必须是有效的帧指针地址
 * @note 如果地址无效，将输出"Cannot backtrace"并返回
 *
 * @par 示例代码
 * @code
 * void exception_handler(uint64_t fp)
 * {
 *     backtrace_r("exception", fp);
 * }
 * @endcode
 *
 * @see backtrace()
 */
void backtrace_r(const char *cookie, uintptr_t frame_address);

#endif /* BACKTRACE_H */
