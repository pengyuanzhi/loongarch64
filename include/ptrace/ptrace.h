/**
 * @file    ptrace.h
 * @brief   ptrace 调试跟踪公共接口
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 本文件定义了 ptrace 调试跟踪的公共接口
 *          - 数据结构定义
 *          - 函数声明
 *          - 常量定义
 *
 * @note MISRA-C:2012 合规
 * @note 此头文件与架构无关，适用于所有平台
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef _PTRACE_H
#define _PTRACE_H

/*************************** 头文件包含 ****************************/
#include <sys/types.h>
#include <sys/user.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*************************** 宏定义 ****************************/
/**
 * @brief 寄存器集合类型（NT_*）
 *
 * @details 用于 ptrace 的 NT_PRSTATUS 等请求
 */
#define NT_PRSTATUS 1    /**< @brief 通用寄存器 */
#define NT_FPREGSET 2    /**< @brief 浮点寄存器 */
#define NT_PRPSINFO 3    /**< @brief 进程信息 */
#define NT_ARM_SVE 0x400 /**< @brief ARM SVE（LoongArch64 不支持） */

/**
 * @brief 用户访问权限标志
 */
#define UACCESS_R 0x1 /**< @brief 读权限 */
#define UACCESS_W 0x2 /**< @brief 写权限 */

    /*************************** 数据结构 ****************************/
    /* user_hwdebug_state 已在 sys/user.h 中定义 */

    /*************************** 类型定义 ****************************/
    /**
     * @brief 进程控制块指针类型（简化版）
     *
     * @details 实际项目中应使用真实的 PCB 结构
     *          这里定义简化版本以供 ptrace 使用
     */
    struct process_control_block
    {
        /* 异常上下文（必须） */
        struct arch_context exception_context;

        /* 进程信息 */
        uint64_t entry;         /**< @brief 程序入口地址 */
        int ptrace_first_start; /**< @brief 首次启动标志 */

        /* ptrace 状态 */
        uint32_t ptrace; /**< @brief ptrace 标志 */

        /* 调试状态 */
        struct
        {
            struct user_hwdebug_state bp; /**< @brief 断点状态 */
            struct user_hwdebug_state wr; /**< @brief 观察点状态 */
            int bp_count;                 /**< @brief 断点数量 */
            int wr_count;                 /**< @brief 观察点数量 */
        } debug_state;
    };

    typedef struct process_control_block *pcb_t;

/*************************** ptrace 标志定义 ****************************/
#define PT_PTRACED 0x00000001U          /**< @brief 被跟踪 */
#define PT_DTRACE 0x00000002U           /**< @brief 延迟跟踪 */
#define PT_TRACESYSGOOD 0x00000004U     /**< @brief 系统调用跟踪良好 */
#define PT_PTRACE_CAP 0x00000008U       /**< @brief ptrace 能力 */
#define PT_TRACE_FORK 0x00000010U       /**< @brief 跟踪 fork */
#define PT_TRACE_VFORK 0x00000020U      /**< @brief 跟踪 vfork */
#define PT_TRACE_CLONE 0x00000040U      /**< @brief 跟踪 clone */
#define PT_TRACE_EXEC 0x00000080U       /**< @brief 跟踪 exec */
#define PT_TRACE_VFORK_DONE 0x00000100U /**< @brief vfork 完成 */
#define PT_TRACE_EXIT 0x00000200U       /**< @brief 跟踪退出 */

#define PT_TRACE_MASK 0x000003F4U /**< @brief 跟踪掩码 */
#define PT_SINGLESTEP 0x80000000U /**< @brief 单步执行 */

    /*************************** 函数声明 ****************************/
    /**
     * @brief 获取用户寄存器
     *
     * @details 获取进程的用户寄存器结构
     *
     * @param pcb 进程控制块指针
     *
     * @return 返回用户寄存器结构指针
     *
     * @note 调用者负责释放返回的内存
     */
    struct user *get_user_regs(pcb_t pcb);

    /**
     * @brief 设置用户寄存器
     *
     * @details 设置进程的用户寄存器值
     *
     * @param pcb  进程控制块指针
     * @param regs 用户寄存器结构指针
     *
     * @return 无
     */
    void set_user_regs(pcb_t pcb, struct user *regs);

    /**
     * @brief 验证用户寄存器
     *
     * @details 验证用户寄存器结构的有效性
     *
     * @param regs 用户寄存器结构指针
     *
     * @return 成功返回0，失败返回负错误码
     *
     * @retval 0 成功
     * @retval -EINVAL 无效的寄存器值
     */
    int valid_user_regs(struct user *regs);

    /**
     * @brief 设置断点
     *
     * @details 为进程设置调试断点
     *
     * @param pcb 子进程控制块指针
     *
     * @return 无
     *
     * @note 启用单步执行模式
     */
    void ptrace_set_bpt(pcb_t pcb);

    /**
     * @brief 取消断点
     *
     * @details 清除进程的所有断点设置
     *
     * @param pcb 进程控制块指针
     *
     * @return 无
     *
     * @note 禁用单步执行模式
     */
    void ptrace_cancel_bpt(pcb_t pcb);

    /**
     * @brief 获取寄存器集合
     *
     * @details 获取进程的寄存器值到用户空间
     *
     * @param pcb    进程控制块指针
     * @param uregs  用户寄存器缓冲区
     * @param nt_type 寄存器集合类型
     *
     * @return 成功返回0，失败返回负错误码
     *
     * @retval 0 成功
     * @retval -EFAULT 内存访问错误
     * @retval -ENOSYS 不支持的操作
     */
    int ptrace_getregset(pcb_t pcb, void *uregs, int nt_type);

    /**
     * @brief 设置寄存器集合
     *
     * @details 从用户空间设置进程的寄存器值
     *
     * @param pcb    进程控制块指针
     * @param uregs  用户寄存器数据
     * @param nt_type 寄存器集合类型
     *
     * @return 成功返回0，失败返回负错误码
     *
     * @retval 0 成功
     * @retval -EFAULT 内存访问错误
     * @retval -EINVAL 无效的参数
     * @retval -ENOSYS 不支持的操作
     */
    int ptrace_setregset(pcb_t pcb, void *uregs, int nt_type);

    /**
     * @brief 重置调试状态
     *
     * @details 清除所有调试相关的状态和设置
     *
     * @param 无
     *
     * @return 无
     *
     * @note 清除所有硬件断点和观察点
     */
    void reset_debug_state(void);

    /**
     * @brief 恢复硬件调试状态
     *
     * @details 从进程控制块恢复硬件调试寄存器的状态
     *
     * @param pcb 进程控制块指针
     *
     * @return 无
     *
     * @note 恢复断点和观察点设置
     */
    void restore_hw_debug(pcb_t pcb);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _PTRACE_H */
