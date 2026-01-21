## 异常处理规范

### 1. 异常处理原则

#### 1.1 安全关键异常处理要求

**必须遵循的原则**：
1. **确定性**：异常处理必须是确定性的，行为可预测
2. **快速响应**：关键异常必须在规定时间内响应
3. **状态保护**：异常发生时必须保护处理器状态
4. **安全恢复**：异常恢复后系统必须处于安全状态
5. **可测试性**：所有异常处理路径必须可测试

#### 1.2 异常处理优先级

```c
/**
 * @brief 异常优先级定义
 * @note 优先级从高到低排列
 */
typedef enum
{
    EXC_PRIORITY_HIGHEST      = 0U,  /* 最高优先级（NMI等） */
    EXC_PRIORITY_TIMER        = 1U,  /* 定时器中断 */
    EXC_PRIORITY_IPI          = 2U,  /* 核心间中断 */
    EXC_PRIORITY_DEVICE       = 3U,  /* 设备中断 */
    EXC_PRIORITY_TLB          = 4U,  /* TLB异常 */
    EXC_PRIORITY_SYSCALL      = 5U,  /* 系统调用 */
    EXC_PRIORITY_FAULT        = 6U,  /* 内存故障 */
    EXC_PRIORITY_TRAP         = 7U,  /* 陷阱/调试 */
    EXC_PRIORITY_LOWEST       = 8U   /* 最低优先级 */
} ExceptionPriority_t;
```

### 2. 异常向量表设置

#### 2.1 异常码定义

**LoongArch64异常码**：

```c
#define EXCCODE_INT_START      64U   /* 中断开始 */

/* TLB异常 */
#define EXCCODE_TLBI          0x1   /* TLB取指缺失 */
#define EXCCODE_TLBL          0x2   /* TLB加载缺失 */
#define EXCCODE_TLBS          0x3   /* TLB存储缺失 */
#define EXCCODE_TLBM          0x4   /* TLB修改错误 */

/* 内存异常 */
#define EXCCODE_ADE           0x8   /* 地址错误 */
#define EXCCODE_ALE           0x9   /* 对齐错误 */

/* 系统调用和异常 */
#define EXCCODE_SYS           0xB   /* 系统调用 */
#define EXCCODE_BP            0xC   /* 断点 */
#define EXCCODE_INE           0xD   /* 指令未实现 */
#define EXCCODE_IPE           0xE   /* 指令特权错误 */
#define EXCCODE_FPDIS         0xF   /* FPU禁用 */
#define EXCCODE_LSXDIS        0x10  /* LSX禁用 */
#define EXCCODE_LASXDIS       0x11  /* LASX禁用 */
#define EXCCODE_FPE           0x12  /* 浮点异常 */

/* 中断结束 */
#define EXCCODE_INT_END       0xBF
```

#### 2.2 异常向量表结构

```c
/**
 * @brief 异常处理函数类型
 */
typedef void (*ExceptionHandler_t)(void);

/**
 * @brief 异常向量表
 * @note 必须对齐到VECSIZE（通常为512字节）
 * @note 使用__attribute__((section()))确保链接到正确段
 */
__attribute__((aligned(512), section(".vectors")))
ExceptionHandler_t exception_table[64] =
{
    /* 保留的异常向量 */
    [0 ... EXCCODE_INT_START - 1] = handle_reserved,

    /* TLB异常 */
    [EXCCODE_TLBI]      = handle_tlb_ifetch,   /* TLB取指缺失 */
    [EXCCODE_TLBL]      = handle_tlb_load,     /* TLB加载缺失 */
    [EXCCODE_TLBS]      = handle_tlb_store,    /* TLB存储缺失 */
    [EXCCODE_TLBM]      = handle_tlb_modify,   /* TLB修改错误 */

    /* 内存异常 */
    [EXCCODE_ADE]       = handle_ade,          /* 地址错误 */
    [EXCCODE_ALE]       = handle_ale,          /* 对齐错误 */

    /* 系统调用和异常 */
    [EXCCODE_SYS]       = handle_syscall,      /* 系统调用 */
    [EXCCODE_BP]        = handle_breakpoint,   /* 断点 */
    [EXCCODE_INE]       = handle_ine,          /* 指令未实现 */
    [EXCCODE_IPE]       = handle_ipe,          /* 指令特权错误 */
    [EXCCODE_FPDIS]     = handle_fpdis,        /* FPU禁用 */

    /* 浮点和SIMD */
    [EXCCODE_LSXDIS]    = handle_lsxdis,       /* LSX禁用 */
    [EXCCODE_LASXDIS]   = handle_lasxdis,      /* LASX禁用 */
    [EXCCODE_FPE]       = handle_fpe,          /* 浮点异常 */

    /* 中断向量（可由硬件动态路由） */
    [EXCCODE_INT_START ... EXCCODE_INT_END] = handle_irq,
};
```

### 3. 异常上下文保存和恢复

#### 3.1 异常上下文结构

```c
/**
 * @brief 异常上下文结构
 * @details 保存异常发生时的处理器状态
 */
typedef struct ExceptionContext
{
    /* 通用寄存器 */
    uint64_t gpr[32];       /* r0-r31 */

    /* 特殊寄存器 */
    uint64_t pc;            /* 程序计数器 */
    uint64_t badv;          /* 出错地址 */
    uint32_t estat;         /* 异常状态寄存器 */
    uint32_t ecode;         /* 异常码 */

    /* 栈信息 */
    uint64_t sp;            /* 栈指针 */

    /* 特权级 */
    uint32_t plv;           /* 特权级（0-3） */

    /* 其他状态 */
    uint32_t flags;         /* 标志位 */

} ExceptionContext_t;
```

#### 3.2 异常入口汇编

```assembly
/**
 * @brief 异常入口汇编代码
 * @note 此函数由硬件直接调用，必须保存完整上下文
 */
.global exception_entry
exception_entry:
    /* 1. 保存关键寄存器到内核栈 */
    addi.d   sp, sp, -256       /* 分配栈空间 */

    /* 保存通用寄存器 */
    st.d     $ra, sp, 0         /* 返回地址 */
    st.d     $a0, sp, 8         /* 参数寄存器 */
    st.d     $a1, sp, 16
    st.d     $a2, sp, 24
    st.d     $a3, sp, 32
    st.d     $a4, sp, 40
    st.d     $a5, sp, 48
    st.d     $a6, sp, 56
    st.d     $a7, sp, 64
    st.d     $t0, sp, 72        /* 临时寄存器 */
    st.d     $t1, sp, 80
    st.d     $t2, sp, 88
    st.d     $t3, sp, 96
    st.d     $t4, sp, 104
    st.d     $t5, sp, 112
    st.d     $t6, sp, 120
    st.d     $t7, sp, 128
    st.d     $fp, sp, 136       /* 帧指针 */
    st.d     $s0, sp, 144       /* 保存寄存器 */
    st.d     $s1, sp, 152
    st.d     $s2, sp, 160
    st.d     $s3, sp, 168
    st.d     $s4, sp, 176
    st.d     $s5, sp, 184
    st.d     $s6, sp, 192
    st.d     $s7, sp, 200
    st.d     $s8, sp, 208

    /* 保存特殊寄存器 */
    csrrd    $t0, 0x6           /* ERA - 异常返回地址 */
    st.d     $t0, sp, 216

    csrrd    $t0, 0x4           /* ECODE - 异常码 */
    st.w     $t0, sp, 224

    /* 2. 调用C语言异常处理函数 */
    move     $a0, sp            /* 参数1：异常上下文指针 */
    bl       exception_handler /* 调用C处理函数 */

    /* 3. 恢复上下文 */
    ld.d     $ra, sp, 0         /* 恢复寄存器 */
    ld.d     $a0, sp, 8
    ld.d     $a1, sp, 16
    /* ... 恢复其他寄存器 ... */

    /* 4. 从异常返回 */
    addi.d   sp, sp, 256        /* 释放栈空间 */
    ertn                        /* 异常返回指令 */
```

#### 3.3 异常处理函数

```c
/**
 * @brief 异常处理函数（C语言）
 * @param ctx 异常上下文指针
 *
 * @note 此函数在异常上下文中执行，必须遵守严格的约束
 */
void exception_handler(ExceptionContext_t *ctx)
{
    /* 提取异常码 */
    uint32_t ecode = ctx->ecode;
    uint32_t estat = ctx->estat;

    /* 根据异常码分发到具体处理函数 */
    if (ecode < EXCCODE_INT_START)
    {
        /* 同步异常 */
        handle_sync_exception(ecode, ctx);
    }
    else
    {
        /* 中断 */
        handle_interrupt(ecode - EXCCODE_INT_START, ctx);
    }
}
```

### 4. TLB异常处理

#### 4.1 TLB缺失处理

```c
/**
 * @brief TLB取指缺失处理
 * @param ctx 异常上下文
 *
 * @note 处理流程：
 * 1. 检查地址是否有效
 * 2. 查找页表
 * 3. 填充TLB
 * 4. 如果失败，触发页错误
 */
void handle_tlb_ifetch(ExceptionContext_t *ctx)
{
    uint64_t badvaddr = ctx->badv;

    /* 检查地址是否在用户空间 */
    if (badvaddr >= USER_SPACE_START)
    {
        /* 用户空间地址 */
        if (!handle_user_page_fault(badvaddr, PFETCH_EXECUTE, ctx))
        {
            /* 页错误处理失败，发送信号给进程 */
            send_signal(ctx, SIGSEGV);
        }
    }
    else
    {
        /* 内核空间地址 */
        if (!handle_kernel_page_fault(badvaddr, PFETCH_EXECUTE))
        {
            /* 内核页错误是致命的 */
            kernel_panic("Kernel page fault at 0x%llx", badvaddr);
        }
    }
}

/**
 * @brief TLB加载/存储缺失处理
 * @param ctx 异常上下文
 * @param is_store 是否为存储操作
 */
void handle_tlb_load_store(ExceptionContext_t *ctx, bool is_store)
{
    uint64_t badvaddr = ctx->badv;
    uint32_t flags = is_store ? PFETCH_WRITE : PFETCH_READ;

    /* 类似于TLB取指缺失的处理 */
    if (badvaddr >= USER_SPACE_START)
    {
        if (!handle_user_page_fault(badvaddr, flags, ctx))
        {
            send_signal(ctx, SIGSEGV);
        }
    }
    else
    {
        if (!handle_kernel_page_fault(badvaddr, flags))
        {
            kernel_panic("Kernel page fault at 0x%llx", badvaddr);
        }
    }
}
```

#### 4.2 TLB填充

```c
/**
 * @brief 填充TLB项
 * @param vaddr 虚拟地址
 * @param paddr 物理地址
 * @param flags 页面标志
 * @param asid 地址空间ID
 * @return 成功返回0，失败返回负错误码
 */
int tlb_fill(uint64_t vaddr, uint64_t paddr, uint32_t flags, uint32_t asid)
{
    uint64_t entryhi, entrylo0, entrylo1;

    /* 构造EntryHi（VPN和ASID） */
    entryhi = ((vaddr >> 12) & 0xFFFFFFFFUL) |  /* VPN */
              ((uint64_t)asid << 48U);           /* ASID */

    /* 构造EntryLo0（PFN和标志） */
    entrylo0 = (paddr & ~0xFFFUL) |  /* PPN */
               (flags & 0x3FU);       /* 标志位 */

    /* EntryLo1不使用（大页映射） */
    entrylo1 = 0;

    /* 写入TLB */
    tlb_write(0, entryhi, entrylo0, entrylo1);

    return 0;
}
```

### 5. 中断处理

#### 5.1 中断分发

```c
/**
 * @brief 中断处理入口
 * @param ctx 异常上下文
 */
void handle_irq(ExceptionContext_t *ctx)
{
    uint32_t estat = ctx->estat;
    uint32_t irq;

    /* 遍历所有中断位 */
    for (irq = 0U; irq < 32U; irq++)
    {
        if (estat & (1U << irq))
        {
            /* 调用具体的中断处理函数 */
            irq_handler_table[irq](ctx);
        }
    }
}

/**
 * @brief 中断处理函数表
 */
typedef void (*IRQHandler_t)(ExceptionContext_t *ctx);

IRQHandler_t irq_handler_table[32] =
{
    [0] = handle_timer_irq,        /* 定时器中断 */
    [1] = handle_ipi_irq,          /* IPI中断 */
    [2] = handle_uart_irq,         /* 串口中断 */
    /* ... 其他中断 ... */
};
```

#### 5.2 定时器中断处理

```c
/**
 * @brief 定时器中断处理
 * @param ctx 异常上下文
 */
void handle_timer_irq(ExceptionContext_t *ctx)
{
    /* 更新系统时间 */
    update_system_time();

    /* 检查当前任务是否需要调度 */
    if (need_reschedule())
    {
        /* 设置调度标志 */
        set_need_reschedule();

        /* 返回后触发调度 */
    }
}
```

#### 5.3 IPI中断处理

```c
/**
 * @brief IPI中断处理
 * @param ctx 异常上下文
 */
void handle_ipi_irq(ExceptionContext_t *ctx)
{
    uint32_t ipi_type = read_ipi_type();

    switch (ipi_type)
    {
        case IPI_RESCHEDULE:
            /* 重新调度 */
            set_need_reschedule();
            break;

        case IPI_STOP:
            /* 停止CPU */
            stop_cpu();
            break;

        case IPI_CALL_FUNC:
            /* 调用函数 */
            execute_ipi_callback();
            break;

        default:
            /* 未知IPI类型 */
            break;
    }

    /* 清除IPI */
    clear_ipi();
}
```

### 6. 系统调用处理

#### 6.1 系统调用入口

```c
/**
 * @brief 系统调用入口
 * @param ctx 异常上下文
 *
 * @note 系统调用号在 $a0 (r4)
 * @note 参数在 $a1-$a5 (r5-r9)
 * @note 返回值写入 $a0 (r4)
 */
void handle_syscall(ExceptionContext_t *ctx)
{
    /* 提取系统调用号和参数 */
    uint64_t syscall_num = ctx->gpr[4];   /* $a0 */
    uint64_t arg1 = ctx->gpr[5];           /* $a1 */
    uint64_t arg2 = ctx->gpr[6];           /* $a2 */
    uint64_t arg3 = ctx->gpr[7];           /* $a3 */
    uint64_t arg4 = ctx->gpr[8];           /* $a4 */
    uint64_t arg5 = ctx->gpr[9];           /* $a5 */

    /* 系统调用表 */
    if (syscall_num < SYSCALL_COUNT)
    {
        /* 调用系统调用处理函数 */
        int64_t ret = syscall_table[syscall_num](arg1, arg2, arg3, arg4, arg5);

        /* 设置返回值 */
        ctx->gpr[4] = (uint64_t)ret;
    }
    else
    {
        /* 无效的系统调用号 */
        ctx->gpr[4] = (uint64_t)(-ENOSYS);
    }
}
```

#### 6.2 系统调用表

```c
/**
 * @brief 系统调用处理函数类型
 */
typedef int64_t (*SyscallHandler_t)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

/**
 * @brief 系统调用表
 */
#define SYSCALL_COUNT   256U

SyscallHandler_t syscall_table[SYSCALL_COUNT] =
{
    [0]  = sys_read,          /* read */
    [1]  = sys_write,         /* write */
    [2]  = sys_open,          /* open */
    [3]  = sys_close,         /* close */
    [4]  = sys_stat,          /* stat */
    [5]  = sys_fstat,         /* fstat */
    [6]  = sys_poll,          /* poll */
    [10] = sys_unlink,        /* unlink */
    [11] = sys_execve,        /* execve */
    [12] = sys_fork,          /* fork */
    [13] = sys_exit,          /* exit */
    /* ... 其他系统调用 ... */
};
```

### 7. 异常栈管理

#### 7.1 异常栈配置

```c
/**
 * @brief 每个CPU的异常栈
 */
#define EXCEPTION_STACK_SIZE  16384U  /* 16KB */

typedef struct
{
    uint8_t stack[EXCEPTION_STACK_SIZE];
} ExceptionStack_t;

/* 每CPU异常栈 */
ExceptionStack_t exception_stacks[MAX_CPUS] __attribute__((aligned(4096)));
```

#### 7.2 异常栈切换

```c
/**
 * @brief 切换到异常栈
 * @param cpu_id CPU编号
 * @return 异常栈顶指针
 */
static inline uint64_t switch_to_exception_stack(uint32_t cpu_id)
{
    ExceptionStack_t *stack = &exception_stacks[cpu_id];

    /* 返回栈顶地址 */
    return (uint64_t)&stack->stack[EXCEPTION_STACK_SIZE];
}
```

### 8. 异常安全规则

#### 8.1 异常上下文中的约束

**在异常处理函数中必须遵守的规则**：

```c
/**
 * @brief 安全的异常处理函数示例
 */
void safe_exception_handler(ExceptionContext_t *ctx)
{
    /* ✅ 正确：使用局部变量 */
    uint32_t local_var = 0;

    /* ✅ 正确：访问栈内存 */
    local_var = process_data();

    /* ✅ 正确：使用自旋锁（短时间） */
    spin_lock(&lock);
    update_shared_data();
    spin_unlock(&lock);

    /* ❌ 错误：使用互斥锁（可能睡眠） */
    /* mutex_lock(&mutex);  禁止！ */

    /* ❌ 错误：调用可能睡眠的函数 */
    /* kmalloc_large();  禁止！ */

    /* ❌ 错误：访问用户空间内存前未验证 */
    /* *user_ptr = value;  禁止！ */

    /* ✅ 正确：验证用户空间指针 */
    if (validate_user_ptr(user_ptr, sizeof(uint32_t)))
    {
        copy_to_user(user_ptr, &value, sizeof(uint32_t));
    }

    /* ❌ 错误：递归异常 */
    /* recursive_call();  禁止！ */

    /* ✅ 正确：调用异步安全的函数 */
    atomic_inc(&counter);
}
```

#### 8.2 异常嵌套处理

```c
/**
 * @brief 异常嵌套计数器
 */
static __thread uint32_t exception_nest_count = 0U;

/**
 * @brief 进入异常处理
 */
static inline void exception_enter(void)
{
    exception_nest_count++;
    barrier();
}

/**
 * @brief 离开异常处理
 */
static inline void exception_exit(void)
{
    exception_nest_count--;
    barrier();
}

/**
 * @brief 检查是否在异常上下文中
 * @return 在异常中返回true，否则返回false
 */
static inline bool in_exception_context(void)
{
    return (exception_nest_count > 0U);
}
```

### 9. 错误处理和恢复

#### 9.1 致命错误处理

```c
/**
 * @brief 内核恐慌（致命错误）
 * @param fmt 格式化字符串
 * @param ... 可变参数
 *
 * @note 此函数不会返回
 */
void kernel_panic(const char *fmt, ...)
{
    /* 禁用所有中断 */
    disable_interrupts();

    /* 打印恐慌信息 */
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    /* 打印调用栈 */
    print_backtrace();

    /* 停止所有CPU */
    stop_all_cpus();

    /* 进入无限循环 */
    for (;;)
    {
        __asm__ volatile("idle 0");
    }
}
```

#### 9.2 异常恢复

```c
/**
 * @brief 从异常恢复
 * @param ctx 异常上下文
 * @return 可以恢复返回true，否则返回false
 */
bool exception_try_recover(ExceptionContext_t *ctx)
{
    uint32_t ecode = ctx->ecode;

    switch (ecode)
    {
        case EXCCODE_TLBI:
        case EXCCODE_TLBL:
        case EXCCODE_TLBS:
            /* TLB异常可以恢复（填充TLB） */
            return true;

        case EXCCODE_ADE:
        case EXCCODE_ALE:
            /* 地址错误通常无法恢复 */
            return false;

        case EXCCODE_FPE:
            /* 浮点异常可能可以恢复 */
            return handle_fpe_exception(ctx);

        default:
            /* 其他异常默认无法恢复 */
            return false;
    }
}
```

---

## 总结

### 必须遵循的原则

1. **快速响应**：异常处理必须快速完成，避免影响系统实时性
2. **状态保护**：完整保存处理器状态，确保可正确恢复
3. **安全第一**：异常处理不得引入安全漏洞
4. **可测试性**：所有异常路径必须可测试
5. **确定性**：异常处理行为必须可预测

### 禁止模式

- ❌ 禁止在异常处理中使用可能睡眠的函数
- ❌ 禁止在异常处理中访问未验证的用户空间内存
- ❌ 禁止递归异常（异常处理函数中再次触发异常）
- ❌ 禁止在异常处理中分配大块内存
- ❌ 禁止忽略致命错误（如内核页错误）
- ❌ 禁止在异常处理中使用浮点运算（未保存FPU状态）

---

**记住**：异常是系统安全的关键防线。正确处理异常是确保系统稳定和安全的基础。
