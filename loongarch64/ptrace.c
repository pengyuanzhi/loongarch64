/**
 * @file    ptrace.c
 * @brief   LoongArch64调试跟踪支持
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件实现调试跟踪（ptrace）功能
 *          - 调试状态管理
 *          - 寄存器访问
 *          - 断点管理
 *
 * @note MISRA-C:2012 合规
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */
/*************************** 头文件包含 ****************************/
#include <arch/loongarch64/arch-uaccess.h>
#include <arch/loongarch64/ptrace.h>
#include <errno.h>
#include <ptrace/ptrace.h>
#include <stdlib.h>
#include <string.h>
/*************************** 类型定义 ****************************/
/**
 * @brief iovec 结构（用于寄存器集合传输）
 *
 * @note 与标准 iovec 类似，用于用户空间和内核空间数据传输
 */
struct iovec
{
    void *iov_base; /**< @brief 缓冲区基地址 */
    size_t iov_len; /**< @brief 缓冲区长度 */
};
/*************************** 内部函数声明 ****************************/
/**
 * @brief 拷贝数据到用户空间
 *
 * @param to 目标地址（用户空间）
 * @param from 源地址（内核空间）
 * @param n 拷贝字节数
 *
 * @return 成功返回0，失败返回非0（拷贝未完成的字节数）
 */
static unsigned long copy_to_user(void *to, const void *from, unsigned long n);

/**
 * @brief 从用户空间拷贝数据
 *
 * @param to 目标地址（内核空间）
 * @param from 源地址（用户空间）
 * @param n 拷贝字节数
 *
 * @return 成功返回0，失败返回非0（拷贝未完成的字节数）
 */
static unsigned long copy_from_user(void *to, const void *from, unsigned long n);
/*************************** 函数实现 ****************************/

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
struct user *get_user_regs(pcb_t pcb)
{
    struct user *regs = NULL;

    if (pcb == NULL)
    {
        return NULL;
    }

    /* 分配内存 */
    regs = (struct user *)malloc(sizeof(struct user));
    if (regs == NULL)
    {
        return NULL;
    }

    memset(regs, 0, sizeof(struct user));

    if (pcb->ptrace_first_start)
    {
        /* 首次启动，使用入口地址 */
        regs->user_regs.csr_era = pcb->entry;
    }
    else
    {
        /* 从异常上下文复制通用寄存器 */
        memcpy(regs->user_regs.regs, pcb->exception_context.regs, sizeof(regs->user_regs.regs));

        /* 复制 CSR 寄存器 */
        regs->user_regs.csr_era = pcb->exception_context.csr_era;
        regs->user_regs.csr_badvaddr = pcb->exception_context.csr_badvaddr;
        regs->user_regs.csr_crmd = pcb->exception_context.csr_crmd;
        regs->user_regs.csr_prmd = pcb->exception_context.csr_prmd;
        regs->user_regs.csr_euen = pcb->exception_context.csr_euen;
        regs->user_regs.csr_ecfg = pcb->exception_context.csr_ecfg;
        regs->user_regs.csr_estat = pcb->exception_context.csr_estat;

        /* TODO: 复制 FPU 状态 */
        /* 注意：arch_context 结构中没有 FPU 上下文 */
        /* 如果需要 FPU 支持，需要从 task_context 中获取 */
    }

    return regs;
}

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
void set_user_regs(pcb_t pcb, struct user *regs)
{
    if ((pcb == NULL) || (regs == NULL))
    {
        return;
    }

    /* 复制通用寄存器 */
    memcpy(&pcb->exception_context.regs[0], regs->user_regs.regs, sizeof(regs->user_regs.regs));

    /* 设置 CSR 寄存器 */
    pcb->exception_context.csr_era = regs->user_regs.csr_era;
    pcb->exception_context.csr_badvaddr = regs->user_regs.csr_badvaddr;
    pcb->exception_context.csr_crmd = regs->user_regs.csr_crmd;
    pcb->exception_context.csr_prmd = regs->user_regs.csr_prmd;
    pcb->exception_context.csr_euen = regs->user_regs.csr_euen;
    pcb->exception_context.csr_ecfg = regs->user_regs.csr_ecfg;
    pcb->exception_context.csr_estat = regs->user_regs.csr_estat;

    /* TODO: 设置 FPU 状态 */
}

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
int valid_user_regs(struct user *regs)
{
    if (regs == NULL)
    {
        return -EINVAL;
    }

    /* 检查特权级别是否为用户模式（PLV3） */
    if ((regs->user_regs.csr_crmd & CRMD_PLV) != 0x3UL)
    {
        return -EINVAL;
    }

    return 0;
}

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
int ptrace_getregset(pcb_t pcb, void *uregs, int nt_type)
{
    struct user *regs = NULL;
    struct iovec *uiov = NULL;
    int ret = 0;

    if (pcb == NULL)
    {
        return -EFAULT;
    }

    regs = get_user_regs(pcb);
    if (regs == NULL)
    {
        return -EFAULT;
    }

    if (nt_type == 0)
    {
        /* 旧式接口：直接返回 user_regs_struct */
        ret = (int)copy_to_user(uregs, &regs->user_regs, sizeof(regs->user_regs));
    }
    else
    {
        /* 新式接口：使用 iovec */
        uiov = (struct iovec *)uregs;

        /* 检查用户空间指针 */
        if (!user_access_check(uiov, sizeof(*uiov), UACCESS_R | UACCESS_W))
        {
            free(regs);
            return -EFAULT;
        }

        switch (nt_type)
        {
        case NT_PRSTATUS:
            /* 通用寄存器 */
            ret = (int)copy_to_user(uiov->iov_base, &regs->user_regs, uiov->iov_len);
            break;

        case NT_FPREGSET:
            /* 浮点寄存器 */
            ret = (int)copy_to_user(uiov->iov_base, &regs->user_fpsimd, uiov->iov_len);
            break;

        default:
            ret = -ENOSYS;
            break;
        }
    }

    free(regs);
    return ret;
}

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
int ptrace_setregset(pcb_t pcb, void *uregs, int nt_type)
{
    struct user *newregs = NULL;
    struct iovec *uiov = NULL;
    int ret = 0;

    if (pcb == NULL)
    {
        return -EFAULT;
    }

    /* 获取当前寄存器作为模板 */
    newregs = get_user_regs(pcb);
    if (newregs == NULL)
    {
        return -EFAULT;
    }

    if (nt_type == 0)
    {
        /* 旧式接口：直接接收 user_regs_struct */
        if (copy_from_user(&newregs->user_regs, uregs, sizeof(newregs->user_regs)) != 0)
        {
            ret = -EFAULT;
        }
        else
        {
            if (valid_user_regs(newregs) == 0)
            {
                set_user_regs(pcb, newregs);
                ret = 0;
            }
            else
            {
                ret = -EINVAL;
            }
        }
    }
    else
    {
        /* 新式接口：使用 iovec */
        uiov = (struct iovec *)uregs;

        /* 检查用户空间指针 */
        if (!user_access_check(uiov, sizeof(*uiov), UACCESS_R))
        {
            ret = -EFAULT;
        }
        else
        {
            switch (nt_type)
            {
            case NT_PRSTATUS:
                /* 通用寄存器 */
                if (copy_from_user(&newregs->user_regs, uiov->iov_base, uiov->iov_len) != 0)
                {
                    ret = -EFAULT;
                }
                else
                {
                    if (valid_user_regs(newregs) == 0)
                    {
                        set_user_regs(pcb, newregs);
                        ret = 0;
                    }
                    else
                    {
                        ret = -EINVAL;
                    }
                }
                break;

            case NT_FPREGSET:
                /* 浮点寄存器 */
                if (copy_from_user(&newregs->user_fpsimd, uiov->iov_base, uiov->iov_len) != 0)
                {
                    ret = -EFAULT;
                }
                else
                {
                    set_user_regs(pcb, newregs);
                    ret = 0;
                }
                break;

            default:
                ret = -ENOSYS;
                break;
            }
        }
    }

    free(newregs);
    return ret;
}

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
void ptrace_set_bpt(pcb_t pcb)
{
    if (pcb == NULL)
    {
        return;
    }

    /* 设置单步调试标志 */
    /* 使用 CRMD 的 DAIE 位进行单步调试 */
    pcb->exception_context.csr_crmd |= CRMD_DAIE;

    /* TODO: 设置硬件断点寄存器（如果需要） */
}

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
void ptrace_cancel_bpt(pcb_t pcb)
{
    if (pcb == NULL)
    {
        return;
    }

    /* 清除单步调试标志 */
    pcb->exception_context.csr_crmd &= ~CRMD_DAIE;

    /* TODO: 清除硬件断点寄存器（如果需要） */
}

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
void reset_debug_state(void)
{
    /* TODO: 清除所有硬件断点和观察点 */
    /* LoongArch64 调试 CSR 寄存器：
     * - PRCTL (0x80): 调试控制寄存器
     * - PRB[0-15] (0xA0-0xAF): 断点地址寄存器
     * - PRM[0-15] (0xB0-0xBF): 断点掩码寄存器
     * - PRB_CTRL[0-15] (0xC0-0xCF): 断点控制寄存器
     */

    /* 清除 PRCTL 寄存器 */
    // write_csr(0x80, 0);

    /* 清除断点寄存器 */
    /* for (int i = 0; i < 16; i++) */
    /* { */
    /*     write_csr(0xA0 + i, 0);     /\* PRB[i] *\/ */
    /*     write_csr(0xC0 + i, 0);     /\* PRB_CTRL[i] *\/ */
    /* } */

    barrier();
}

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
void restore_hw_debug(pcb_t pcb)
{
    int i;

    if (pcb == NULL)
    {
        return;
    }

    /* TODO: 从 PCB 恢复硬件断点和观察点 */
    /* 恢复断点设置 */
    for (i = 0; i < pcb->debug_state.bp_count; i++)
    {
        /* 恢复断点寄存器（伪代码，需要实际 CSR 定义） */
        /* write_dbg_bpr(i, pcb->debug_state.bp.dbg_regs[i].ctrl); */
        /* write_dbg_bvr(i, pcb->debug_state.bp.dbg_regs[i].addr); */
    }

    /* 恢复观察点设置 */
    for (i = 0; i < pcb->debug_state.wr_count; i++)
    {
        /* 恢复观察点寄存器 */
        /* write_dbg_wpr(i, pcb->debug_state.wr.dbg_regs[i].ctrl); */
        /* write_dbg_wvr(i, pcb->debug_state.wr.dbg_regs[i].addr); */
    }

    barrier();
}

/*************************** 内部函数实现 ****************************/

/**
 * @brief 拷贝数据到用户空间
 *
 * @details 将数据从内核空间拷贝到用户空间
 *          先检查用户空间地址有效性，再执行拷贝
 *
 * @param to 目标地址（用户空间）
 * @param from 源地址（内核空间）
 * @param n 拷贝字节数
 *
 * @return 成功返回0，失败返回非0（拷贝未完成的字节数）
 */
static unsigned long copy_to_user(void *to, const void *from, unsigned long n)
{
    /* 检查用户空间地址有效性 */
    if (!user_access_check(to, n, UACCESS_W))
    {
        return n;
    }

    /* 执行拷贝 */
    memcpy(to, from, n);

    return 0;
}

/**
 * @brief 从用户空间拷贝数据
 *
 * @details 将数据从用户空间拷贝到内核空间
 *          先检查用户空间地址有效性，再执行拷贝
 *
 * @param to 目标地址（内核空间）
 * @param from 源地址（用户空间）
 * @param n 拷贝字节数
 *
 * @return 成功返回0，失败返回非0（拷贝未完成的字节数）
 */
static unsigned long copy_from_user(void *to, const void *from, unsigned long n)
{
    /* 检查用户空间地址有效性 */
    if (!user_access_check(from, n, UACCESS_R))
    {
        return n;
    }

    /* 执行拷贝 */
    memcpy(to, from, n);

    return 0;
}
