/**
 * @file    ptrace.h
 * @brief   标准 Linux ptrace 用户空间接口
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 本文件定义符合 Linux 标准的 ptrace 用户空间接口
 *          - ptrace 请求类型定义
 *          - ptrace 函数声明
 *          - 兼容 Linux sys/ptrace.h
 *
 * @note 符合 Linux 标准接口
 * @note 与 Linux glibc ptrace 接口兼容
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef _SYS_PTRACE_H
#define _SYS_PTRACE_H

/*************************** 头文件包含 ****************************/
#include <system/types.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*************************** 宏定义 ****************************/
/**
 * @brief PTRACE_PEEKUSER/POKEUSER 的偏移量
 *
 * @details 这些偏移量对应 struct user_regs_struct 中的字段
 */
#define PTRACE_TEXT_OFFSET 0U /**< @brief 代码段偏移 */
#define PTRACE_DATA_OFFSET 0U /**< @brief 数据段偏移 */

/**
 * @defgroup NT_Types NT_PRSTATUS 寄存器集合类型
 * @brief 用于 GETREGSET/SETREGSET 的寄存器集合类型
 * @{
 */
#ifndef NT_PRSTATUS
#define NT_PRSTATUS 1 /**< @brief 通用寄存器 */
#endif

#ifndef NT_FPREGSET
#define NT_FPREGSET 2 /**< @brief 浮点寄存器 */
#endif

#ifndef NT_PRPSINFO
#define NT_PRPSINFO 3 /**< @brief 进程信息 */
#endif

#ifndef NT_X86_XSTATE
#define NT_X86_XSTATE 0x202 /**< @brief x86 XSTATE（LoongArch64 不支持） */
#endif
/** @} */

/*************************** 枚举定义 ****************************/
/**
 * @brief ptrace 请求类型定义（Linux 标准）
 *
 * @details 这些定义与 Linux kernel uapi/linux/ptrace.h 一致
 */
enum __ptrace_request
{
    PTRACE_TRACEME = 0,    /**< @brief 本进程被父进程跟踪 */
    PTRACE_PEEKTEXT = 1,   /**< @brief 从内存读取字 */
    PTRACE_PEEKDATA = 2,   /**< @brief 从内存读取字（同PEEKTEXT） */
    PTRACE_PEEKUSER = 3,   /**< @brief 读取 USER 区域（寄存器） */
    PTRACE_POKETEXT = 4,   /**< @brief 向内存写入字 */
    PTRACE_POKEDATA = 5,   /**< @brief 向内存写入字（同POKETEXT） */
    PTRACE_POKEUSER = 6,   /**< @brief 向 USER 区域写入（寄存器） */
    PTRACE_CONT = 7,       /**< @brief 继续执行 */
    PTRACE_KILL = 8,       /**< @brief 终止进程 */
    PTRACE_SINGLESTEP = 9, /**< @brief 单步执行 */

    PTRACE_GETREGS = 12,   /**< @brief 获取通用寄存器 */
    PTRACE_SETREGS = 13,   /**< @brief 设置通用寄存器 */
    PTRACE_GETFPREGS = 14, /**< @brief 获取浮点寄存器 */
    PTRACE_SETFPREGS = 15, /**< @brief 设置浮点寄存器 */
    PTRACE_ATTACH = 16,    /**< @brief 附加到进程 */
    PTRACE_DETACH = 17,    /**< @brief 从进程分离 */

    PTRACE_GETREGSET = 0x4204, /**< @brief 获取寄存器集合（新接口） */
    PTRACE_SETREGSET = 0x4205, /**< @brief 设置寄存器集合（新接口） */

    PTRACE_SEIZE = 0x4206,     /**< @brief 占用进程（不停止它） */
    PTRACE_INTERRUPT = 0x4207, /**< @brief 中断进程 */
    PTRACE_LISTEN = 0x4208,    /**< @brief 监听停止的进程 */

    PTRACE_GETSIGINFO = 0x4209, /**< @brief 获取 siginfo */
    PTRACE_SETSIGINFO = 0x420a, /**< @brief 设置 siginfo */
    PTRACE_GETSIGMASK = 0x420b, /**< @brief 获取信号掩码 */
    PTRACE_SETSIGMASK = 0x420c, /**< @brief 设置信号掩码 */
};

/*************************** 函数声明 ****************************/
/**
 * @brief ptrace 系统调用接口
 *
 * @details 提供进程跟踪和调试功能
 *
 * @param request ptrace 请求类型
 * @param pid 目标进程 ID
 * @param addr 地址（根据请求类型不同含义不同）
 * @param data 数据（根据请求类型不同含义不同）
 *
 * @return 成功返回请求数据或0，失败返回-1并设置errno
 *
 * @note 这是标准 Linux ptrace 接口
 *
 * @par 示例代码
 * @code
 * // 读取子进程的寄存器
 * struct user_regs_struct regs;
 * if (ptrace(PTRACE_GETREGS, pid, NULL, &regs) == -1) {
 *     perror("ptrace GETREGS");
 *     return -1;
 * }
 * @endcode
 */
long ptrace(enum __ptrace_request request, pid_t pid, void *addr, void *data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _SYS_PTRACE_H */
