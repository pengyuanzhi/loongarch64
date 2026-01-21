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
#include <sys/ptrace.h>
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

/*************************** 进程查找辅助函数 ****************************/

/**
 * @brief 根据 PID 查找进程控制块
 *
 * @details 从进程表中查找指定 PID 的 PCB
 *
 * @param pid 进程 ID
 *
 * @return 成功返回 PCB 指针，失败返回 NULL
 *
 * @note TODO: 需要连接到实际的任务管理系统
 */
static pcb_t find_pcb_by_pid(pid_t pid)
{
    /* TODO: 实际实现需要从任务管理系统中查找 */
    /*
     * 参考实现伪代码：
     *
     * if (pid == 0) {
     *     return current_pcb;  // 当前进程
     * }
     *
     * struct task_struct *task = find_task_by_vpid(pid);
     * if (task == NULL) {
     *     return NULL;
     * }
     *
     * return task->pcb;
     */

    (void)pid;
    return NULL; /* 临时返回 NULL，实际使用时需要实现 */
}

/**
 * @brief 获取当前进程的 PCB
 *
 * @return 返回当前进程的 PCB 指针
 *
 * @note TODO: 需要连接到实际的任务管理系统
 */
static pcb_t get_current_pcb(void)
{
    /* TODO: 实际实现需要从任务管理系统中获取当前进程 */
    /*
     * 参考实现伪代码：
     *
     * TASK_ID tid = ttosGetRunningTask();
     * return get_pcb_by_task_id(tid);
     */

    return NULL; /* 临时返回 NULL，实际使用时需要实现 */
}

/*************************** 内存访问辅助函数 ****************************/

/**
 * @brief 从目标进程内存读取数据
 *
 * @details 从指定进程的虚拟地址空间读取数据
 *
 * @param pcb 目标进程 PCB
 * @param addr 目标虚拟地址
 * @param data 存储读取数据的缓冲区
 * @param size 读取字节数
 *
 * @return 成功返回0，失败返回负错误码
 */
static int ptrace_read_mem(pcb_t pcb, void *addr, void *data, size_t size)
{
    if ((pcb == NULL) || (addr == NULL) || (data == NULL))
    {
        return -EINVAL;
    }

    /* TODO: 实现跨进程内存读取 */
    /*
     * 参考实现伪代码：
     *
     * 1. 检查目标地址是否在用户空间
     * 2. 切换到目标进程的地址空间
     * 3. 执行内存读取
     * 4. 恢复当前进程的地址空间
     *
     * if (!access_ok(VERIFY_READ, addr, size)) {
     *     return -EFAULT;
     * }
     *
     * old_mm = current->mm;
     * current->mm = pcb->mm;
     * barrier();
     *
     * ret = copy_from_user_kernel(data, addr, size);
     *
     * current->mm = old_mm;
     * barrier();
     */

    (void)pcb;
    (void)size;
    return -ENOSYS; /* 暂不支持 */
}

/**
 * @brief 向目标进程内存写入数据
 *
 * @details 向指定进程的虚拟地址空间写入数据
 *
 * @param pcb 目标进程 PCB
 * @param addr 目标虚拟地址
 * @param data 要写入的数据
 * @param size 写入字节数
 *
 * @return 成功返回0，失败返回负错误码
 */
static int ptrace_write_mem(pcb_t pcb, void *addr, const void *data, size_t size)
{
    if ((pcb == NULL) || (addr == NULL) || (data == NULL))
    {
        return -EINVAL;
    }

    /* TODO: 实现跨进程内存写入 */
    /* 参考实现同 ptrace_read_mem */

    (void)pcb;
    (void)addr;
    (void)data;
    (void)size;
    return -ENOSYS; /* 暂不支持 */
}

/*************************** PEEKUSER/POKEUSER 辅助函数 ****************************/

/**
 * @brief 根据 offset 获取寄存器值
 *
 * @details 按照 Linux PTRACE_PEEKUSER 的偏移量约定获取寄存器值
 *
 * @param regs 用户寄存器结构
 * @param offset 偏移量（以字节为单位）
 * @param data 存储返回值
 *
 * @return 成功返回0，失败返回负错误码
 */
static int ptrace_peek_user(struct user *regs, unsigned int offset, unsigned long *data)
{
    if ((regs == NULL) || (data == NULL))
    {
        return -EINVAL;
    }

    /* 偏移量到寄存器的映射 */
    /* 基于 struct user_regs_struct 的布局 */
    if (offset < (LOONGARCH64_NUM_REGS * sizeof(uint64_t)))
    {
        /* 通用寄存器 r0-r31 */
        int reg_index = (int)(offset / sizeof(uint64_t));
        if (reg_index < LOONGARCH64_NUM_REGS)
        {
            *data = regs->user_regs.regs[reg_index];
            return 0;
        }
    }
    else if (offset == (LOONGARCH64_pc * sizeof(uint64_t)))
    {
        /* PC (csr_era) */
        *data = regs->user_regs.csr_era;
        return 0;
    }
    else if (offset == (LOONGARCH64_badvaddr * sizeof(uint64_t)))
    {
        /* BadVAddr */
        *data = regs->user_regs.csr_badvaddr;
        return 0;
    }
    else if (offset == (LOONGARCH64_crmd * sizeof(uint64_t)))
    {
        /* CRMD */
        *data = regs->user_regs.csr_crmd;
        return 0;
    }
    else if (offset == (LOONGARCH64_prmd * sizeof(uint64_t)))
    {
        /* PRMD */
        *data = regs->user_regs.csr_prmd;
        return 0;
    }
    else if (offset == (LOONGARCH64_euen * sizeof(uint64_t)))
    {
        /* EUEN */
        *data = regs->user_regs.csr_euen;
        return 0;
    }
    else if (offset == (LOONGARCH64_ecfg * sizeof(uint64_t)))
    {
        /* ECFG */
        *data = regs->user_regs.csr_ecfg;
        return 0;
    }
    else if (offset == (LOONGARCH64_estat * sizeof(uint64_t)))
    {
        /* ESTAT */
        *data = regs->user_regs.csr_estat;
        return 0;
    }

    return -EIO; /* 无效的偏移量 */
}

/**
 * @brief 根据 offset 设置寄存器值
 *
 * @details 按照 Linux PTRACE_POKEUSER 的偏移量约定设置寄存器值
 *
 * @param regs 用户寄存器结构
 * @param offset 偏移量（以字节为单位）
 * @param data 要设置的值
 *
 * @return 成功返回0，失败返回负错误码
 */
static int ptrace_poke_user(struct user *regs, unsigned int offset, unsigned long data)
{
    if (regs == NULL)
    {
        return -EINVAL;
    }

    /* 偏移量到寄存器的映射 */
    if (offset < (LOONGARCH64_NUM_REGS * sizeof(uint64_t)))
    {
        /* 通用寄存器 r0-r31 */
        int reg_index = (int)(offset / sizeof(uint64_t));
        if (reg_index < LOONGARCH64_NUM_REGS)
        {
            regs->user_regs.regs[reg_index] = data;
            return 0;
        }
    }
    else if (offset == (LOONGARCH64_pc * sizeof(uint64_t)))
    {
        /* PC (csr_era) */
        regs->user_regs.csr_era = data;
        return 0;
    }
    else if (offset == (LOONGARCH64_badvaddr * sizeof(uint64_t)))
    {
        /* BadVAddr (只读) */
        return -EIO;
    }
    else if (offset == (LOONGARCH64_crmd * sizeof(uint64_t)))
    {
        /* CRMD */
        regs->user_regs.csr_crmd = data;
        return 0;
    }
    else if (offset == (LOONGARCH64_prmd * sizeof(uint64_t)))
    {
        /* PRMD */
        regs->user_regs.csr_prmd = data;
        return 0;
    }
    else if (offset == (LOONGARCH64_euen * sizeof(uint64_t)))
    {
        /* EUEN */
        regs->user_regs.csr_euen = data;
        return 0;
    }
    else if (offset == (LOONGARCH64_ecfg * sizeof(uint64_t)))
    {
        /* ECFG */
        regs->user_regs.csr_ecfg = data;
        return 0;
    }
    else if (offset == (LOONGARCH64_estat * sizeof(uint64_t)))
    {
        /* ESTAT */
        regs->user_regs.csr_estat = data;
        return 0;
    }

    return -EIO; /* 无效的偏移量 */
}

/*************************** 标准 Linux ptrace 接口实现 ****************************/

/**
 * @brief 标准 Linux ptrace 系统调用接口
 *
 * @details 实现符合 Linux 标准的 ptrace 功能
 *          支持所有标准的 ptrace 请求类型
 *
 * @param request ptrace 请求类型
 * @param pid 目标进程 ID
 * @param addr 地址（根据请求类型不同含义不同）
 * @param data 数据（根据请求类型不同含义不同）
 *
 * @return 成功返回请求数据或0，失败返回负错误码
 *
 * @retval 正数 PEEK 操作返回的数据
 * @retval 0 成功
 * @retval 负数 失败（错误码）
 */
long sys_ptrace(enum __ptrace_request request, pid_t pid, void *addr, void *data)
{
    pcb_t target_pcb = NULL;
    pcb_t current_pcb = NULL;
    long ret = 0;
    struct user *regs = NULL;
    unsigned long peek_data;
    unsigned int offset;
    int tmp_errno;

    /* 根据 pid 查找目标进程 */
    if (pid == 0)
    {
        /* pid == 0 表示当前进程 */
        current_pcb = get_current_pcb();
        if (request == PTRACE_TRACEME)
        {
            target_pcb = current_pcb;
        }
        else
        {
            /* 非 TRACEME 请求，pid=0 表示当前进程 */
            target_pcb = current_pcb;
        }
    }
    else
    {
        /* 查找指定 pid 的进程 */
        target_pcb = find_pcb_by_pid(pid);
        if (target_pcb == NULL)
        {
            return -ESRCH;
        }
    }

    switch (request)
    {
    case PTRACE_TRACEME:
        /* 本进程被父进程跟踪 */
        if (current_pcb == NULL)
        {
            ret = -ESRCH;
            break;
        }
        /* 设置当前进程的 ptrace 标志 */
        current_pcb->ptrace |= PT_PTRACED;
        ret = 0;
        break;

    case PTRACE_PEEKTEXT:
    case PTRACE_PEEKDATA:
        /* 从内存读取字 */
        /* addr 是目标地址，data 是返回数据的指针 */
        if ((addr == NULL) || (data == NULL))
        {
            ret = -EFAULT;
            break;
        }

        /* 从目标进程读取一个字 */
        ret = (long)ptrace_read_mem(target_pcb, addr, &peek_data, sizeof(unsigned long));
        if (ret == 0)
        {
            /* 将读取的数据拷贝到用户空间 */
            *(unsigned long *)data = peek_data;
            ret = peek_data; /* PEEK 操作返回读取的数据 */
        }
        break;

    case PTRACE_PEEKUSER:
        /* 读取 USER 区域（寄存器） */
        /* addr 是寄存器偏移，data 是返回数据的指针 */
        if (data == NULL)
        {
            ret = -EFAULT;
            break;
        }

        /* 转换 addr 为偏移量 */
        offset = (unsigned int)(uintptr_t)addr;

        /* 获取目标进程的寄存器 */
        regs = get_user_regs(target_pcb);
        if (regs == NULL)
        {
            ret = -EFAULT;
            break;
        }

        /* 根据偏移量读取对应的寄存器 */
        tmp_errno = ptrace_peek_user(regs, offset, &peek_data);
        if (tmp_errno == 0)
        {
            *(unsigned long *)data = peek_data;
            ret = peek_data; /* PEEK 操作返回读取的数据 */
        }
        else
        {
            ret = tmp_errno;
        }

        free(regs);
        break;

    case PTRACE_POKETEXT:
    case PTRACE_POKEDATA:
        /* 向内存写入字 */
        /* addr 是目标地址，data 是要写入的数据 */
        if (addr == NULL)
        {
            ret = -EFAULT;
            break;
        }

        /* 向目标进程写入一个字 */
        peek_data = (unsigned long)(uintptr_t)data;
        ret = (long)ptrace_write_mem(target_pcb, addr, &peek_data, sizeof(unsigned long));
        break;

    case PTRACE_POKEUSER:
        /* 向 USER 区域写入（寄存器） */
        /* addr 是寄存器偏移，data 是要写入的数据 */
        if (data == NULL)
        {
            ret = -EFAULT;
            break;
        }

        /* 转换 addr 和 data */
        offset = (unsigned int)(uintptr_t)addr;
        peek_data = (unsigned long)(uintptr_t)data;

        /* 获取目标进程的寄存器 */
        regs = get_user_regs(target_pcb);
        if (regs == NULL)
        {
            ret = -EFAULT;
            break;
        }

        /* 根据偏移量设置对应的寄存器 */
        ret = (long)ptrace_poke_user(regs, offset, peek_data);
        if (ret == 0)
        {
            /* 验证并设置寄存器 */
            if (valid_user_regs(regs) == 0)
            {
                set_user_regs(target_pcb, regs);
            }
            else
            {
                ret = -EINVAL;
            }
        }

        free(regs);
        break;

    case PTRACE_CONT:
        /* 继续执行 */
        /* 清除单步调试标志 */
        if (target_pcb != NULL)
        {
            ptrace_cancel_bpt(target_pcb);
        }
        ret = 0;
        break;

    case PTRACE_KILL:
        /* 终止进程 */
        /* TODO: 发送 SIGKILL 信号给目标进程 */
        /* 临时实现：停止进程 */
        if (target_pcb != NULL)
        {
            ptrace_cancel_bpt(target_pcb);
            /* kill_task(target_pcb); */
        }
        ret = -ENOSYS; /* 需要信号支持 */
        break;

    case PTRACE_SINGLESTEP:
        /* 单步执行 */
        if (target_pcb != NULL)
        {
            ptrace_set_bpt(target_pcb);
        }
        ret = 0;
        break;

    case PTRACE_GETREGS:
        /* 获取通用寄存器（旧接口） */
        if (data == NULL)
        {
            ret = -EFAULT;
            break;
        }

        if (target_pcb == NULL)
        {
            ret = -ESRCH;
            break;
        }

        regs = get_user_regs(target_pcb);
        if (regs == NULL)
        {
            ret = -EFAULT;
            break;
        }

        ret = (long)copy_to_user(data, &regs->user_regs, sizeof(regs->user_regs));
        free(regs);
        break;

    case PTRACE_SETREGS:
        /* 设置通用寄存器（旧接口） */
        if (data == NULL)
        {
            ret = -EFAULT;
            break;
        }

        if (target_pcb == NULL)
        {
            ret = -ESRCH;
            break;
        }

        regs = get_user_regs(target_pcb);
        if (regs == NULL)
        {
            ret = -EFAULT;
            break;
        }

        if (copy_from_user(&regs->user_regs, data, sizeof(regs->user_regs)) != 0)
        {
            ret = -EFAULT;
        }
        else if (valid_user_regs(regs) == 0)
        {
            set_user_regs(target_pcb, regs);
            ret = 0;
        }
        else
        {
            ret = -EINVAL;
        }

        free(regs);
        break;

    case PTRACE_GETFPREGS:
        /* 获取浮点寄存器（旧接口） */
        if (data == NULL)
        {
            ret = -EFAULT;
            break;
        }

        if (target_pcb == NULL)
        {
            ret = -ESRCH;
            break;
        }

        regs = get_user_regs(target_pcb);
        if (regs == NULL)
        {
            ret = -EFAULT;
            break;
        }

        ret = (long)copy_to_user(data, &regs->user_fpsimd, sizeof(regs->user_fpsimd));
        free(regs);
        break;

    case PTRACE_SETFPREGS:
        /* 设置浮点寄存器（旧接口） */
        if (data == NULL)
        {
            ret = -EFAULT;
            break;
        }

        if (target_pcb == NULL)
        {
            ret = -ESRCH;
            break;
        }

        regs = get_user_regs(target_pcb);
        if (regs == NULL)
        {
            ret = -EFAULT;
            break;
        }

        if (copy_from_user(&regs->user_fpsimd, data, sizeof(regs->user_fpsimd)) != 0)
        {
            ret = -EFAULT;
        }
        else
        {
            set_user_regs(target_pcb, regs);
            ret = 0;
        }

        free(regs);
        break;

    case PTRACE_GETREGSET:
        /* 获取寄存器集合（新接口） */
        /* addr 是 iovec 指针，data 是 nt_type */
        if (addr == NULL)
        {
            ret = -EFAULT;
            break;
        }

        if (target_pcb == NULL)
        {
            ret = -ESRCH;
            break;
        }

        /* data 参数在 GETREGSET/SETREGSET 中是 nt_type */
        ret = ptrace_getregset(target_pcb, addr, (int)(uintptr_t)data);
        break;

    case PTRACE_SETREGSET:
        /* 设置寄存器集合（新接口） */
        /* addr 是 iovec 指针，data 是 nt_type */
        if (addr == NULL)
        {
            ret = -EFAULT;
            break;
        }

        if (target_pcb == NULL)
        {
            ret = -ESRCH;
            break;
        }

        ret = ptrace_setregset(target_pcb, addr, (int)(uintptr_t)data);
        break;

    case PTRACE_ATTACH:
        /* 附加到进程 */
        /* TODO: 发送 SIGSTOP 给目标进程并附加 */
        /* 临时实现：直接设置 ptrace 标志 */
        if (target_pcb == NULL)
        {
            ret = -ESRCH;
            break;
        }
        target_pcb->ptrace |= PT_PTRACED;
        /* send_signal(target_pcb, SIGSTOP); */
        ret = 0;
        break;

    case PTRACE_DETACH:
        /* 从进程分离 */
        if (target_pcb != NULL)
        {
            ptrace_cancel_bpt(target_pcb);
            /* 清除 ptrace 标志并恢复进程运行 */
            target_pcb->ptrace &= ~PT_PTRACED;
            /* wake_up_process(target_pcb); */
        }
        ret = 0;
        break;

    case PTRACE_SEIZE:
        /* 占用进程（不停止它） */
        /* 类似 ATTACH 但不发送 SIGSTOP */
        if (target_pcb == NULL)
        {
            ret = -ESRCH;
            break;
        }
        target_pcb->ptrace |= PT_PTRACED;
        ret = 0;
        break;

    case PTRACE_INTERRUPT:
        /* 中断进程 */
        /* TODO: 停止进程并等待 ptrace 命令 */
        ret = -ENOSYS;
        break;

    case PTRACE_LISTEN:
        /* 监听停止的进程 */
        /* TODO: 等待进程停止 */
        ret = -ENOSYS;
        break;

    case PTRACE_GETSIGINFO:
    case PTRACE_SETSIGINFO:
    case PTRACE_GETSIGMASK:
    case PTRACE_SETSIGMASK:
        /* 信号相关操作（暂不支持） */
        ret = -ENOSYS;
        break;

    default:
        ret = -EINVAL;
        break;
    }

    return ret;
}

/**
 * @brief 用户空间 ptrace 接口（调用系统调用）
 *
 * @details 这是用户空间调用的 ptrace 函数，通过系统调用进入内核
 *
 * @param request ptrace 请求类型
 * @param pid 目标进程 ID
 * @param addr 地址
 * @param data 数据
 *
 * @return 成功返回请求数据或0，失败返回-1并设置errno
 */
long ptrace(enum __ptrace_request request, pid_t pid, void *addr, void *data)
{
    /* TODO: 执行系统调用 sys_ptrace */
    /* return syscall(__NR_ptrace, request, pid, addr, data); */

    /* 临时：直接调用内核函数 */
    return sys_ptrace(request, pid, addr, data);
}
