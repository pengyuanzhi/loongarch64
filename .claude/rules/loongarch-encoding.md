## LoongArch64特定编码规范

### 1. 数据类型规范

#### 1.1 标准数据类型

```c
/* 固定宽度整数类型（必须使用） */
#include <stdint.h>

int8_t    i8;    /* 8位有符号整数 */
uint8_t   u8;    /* 8位无符号整数 */
int16_t   i16;   /* 16位有符号整数 */
uint16_t  u16;   /* 16位无符号整数 */
int32_t   i32;   /* 32位有符号整数 */
uint32_t  u32;   /* 32位无符号整数 */
int64_t   i64;   /* 64位有符号整数 */
uint64_t  u64;   /* 64位无符号整数 */

/* 指针大小整数类型 */
uintptr_t  ptr_value;  /* 可存放指针的无符号整数 */
intptr_t   ptr_signed; /* 可存放指针的有符号整数 */

/* 最大/最小宽度整数类型 */
intmax_t   max_int;
uintmax_t  max_uint;
```

#### 1.2 类型定义命名规范

```c
/* 结构体和联合体必须使用typedef */
typedef struct TaskControlBlock TCB_t;
typedef struct Mutex Mutex_t;
typedef struct PageTable PageTable_t;

/* 函数指针类型定义 */
typedef void (*TaskEntry_t)(void);
typedef uint32_t (*ErrorHandler_t)(uint32_t error_code);

/* 枚举类型定义 */
typedef enum
{
    TASK_READY = 0U,      /* 就绪态：等待CPU调度 */
    TASK_RUNNING,         /* 运行态：正在执行 */
    TASK_BLOCKED,         /* 阻塞态：等待资源（信号量、消息队列） */
    TASK_SLEEPING,        /* 休眠态：延时等待，超时自动唤醒 */
    TASK_SUSPENDED        /* 挂起态：被挂起，需要显式恢复 */
} TaskState_t;
```

### 2. 对齐规范

#### 2.1 数据对齐

```c
/* 16字节对齐（SIMD优化） */
typedef struct
{
    uint64_t data[2];
} __attribute__((aligned(16))) SIMDData_t;

/* 缓存行对齐（64字节，多核共享数据） */
typedef struct
{
    atomic_uint64_t lock;
    uint64_t data[7];
} __attribute__((aligned(64))) CacheLine_t;

/* 页对齐（4KB） */
typedef struct
{
    uint64_t entries[512];
} __attribute__((aligned(4096))) PageTable_t;
```

#### 2.2 栈对齐

```c
/* 函数入口必须16字节对齐（LoongArch64 ABI要求） */
void task_entry(void)
{
    /* 栈指针保证16字节对齐 */
}

/* 分配栈时确保16字节对齐 */
uint64_t *stack_alloc(uint32_t size)
{
    uint64_t *stack = malloc(size + 15U);
    if (stack != NULL)
    {
        stack = (uint64_t *)(((uintptr_t)stack + 15U) & ~0xFU);
    }
    return stack;
}
```

### 3. 内联汇编规范

#### 3.1 基本内联汇编规则

**必须遵循的原则**：
1. 使用 `volatile` 关键字防止优化
2. 明确指定输入/输出操作数约束
3. 正确声明clobber列表（破坏的寄存器/内存）
4. 使用CSR编号而非宏名称（提高可移植性）

```c
/* ✅ 正确：使用volatile和正确的约束 */
static inline void memory_barrier(void)
{
    __asm__ volatile("dbar 0" ::: "memory");
}

/* ✅ 正确：带输入输出的内联汇编 */
static inline uint64_t get_cycle_count(void)
{
    uint64_t count;
    __asm__ volatile("rdtime.d %0, $zero" : "=r"(count));
    return count;
}

/* ❌ 错误：缺少volatile关键字 */
static inline void wrong_barrier(void)
{
    __asm__("dbar 0" ::: "memory");  /* 可能被优化掉 */
}
```

#### 3.2 CSR（控制和状态寄存器）操作

**常用CSR寄存器编号**：

| CSR编号 | 名称 | 用途 |
|---------|------|------|
| 0x0 | CRMD | 当前模式寄存器 |
| 0x1 | PRMD | 前一模式寄存器 |
| 0x2 | EUEN | 扩展单元使能 |
| 0x3 | MISCFG | 错误配置寄存器 |
| 0x4 | ECODE | 异常码 |
| 0x5 | ESTAT | 异常状态 |
| 0x6 | ERA | 异常返回地址 |
| 0x19 | PGDL | 页表基地址（低地址） |
| 0x1A | PGDH | 页表基地址（高地址） |
| 0x1B | PGD | 页表基地址 |
| 0x180 | CPUID | CPU标识 |
| 0x1048 | IPI_SEND | IPI发送寄存器 |

**CSR读写操作**：

```c
/**
 * @brief 读取CSR寄存器
 * @param csr_num CSR编号
 * @return CSR寄存器值
 */
static inline uint64_t read_csr(uint32_t csr_num)
{
    uint64_t value;
    __asm__ volatile("csrrd %0, %1" : "=r"(value) : "i"(csr_num));
    return value;
}

/**
 * @brief 写入CSR寄存器
 * @param csr_num CSR编号
 * @param value 写入值
 */
static inline void write_csr(uint32_t csr_num, uint64_t value)
{
    __asm__ volatile("csrwr %0, %1" :: "r"(value), "i"(csr_num));
    barrier_inst();  /* 确保指令同步 */
}

/**
 * @brief 交换CSR寄存器值（读取并写入新值）
 * @param csr_num CSR编号
 * @param value 新值
 * @return 原来的CSR值
 */
static inline uint64_t swap_csr(uint32_t csr_num, uint64_t value)
{
    uint64_t old;
    __asm__ volatile("csrwr %0, %2" : "=r"(old) : "0"(value), "i"(csr_num));
    barrier_inst();
    return old;
}

/**
 * @brief 读取并清除CSR寄存器特定位
 * @param csr_num CSR编号
 * @param mask 需要清除的位掩码
 * @return 原来的CSR值
 */
static inline uint64_t read_clear_csr(uint32_t csr_num, uint64_t mask)
{
    uint64_t old;
    __asm__ volatile(
        "csrrd %0, %2\n"
        "andi  %1, %0, %3\n"
        "csrwr %1, %2\n"
        : "=&r"(old), "=&r"(mask)
        : "i"(csr_num), "r"(~mask)
    );
    barrier_inst();
    return old;
}

/**
 * @brief 读取并设置CSR寄存器特定位
 * @param csr_num CSR编号
 * @param mask 需要设置的位掩码
 * @return 原来的CSR值
 */
static inline uint64_t read_set_csr(uint32_t csr_num, uint64_t mask)
{
    uint64_t old;
    __asm__ volatile(
        "csrrd %0, %2\n"
        "or    %1, %0, %3\n"
        "csrwr %1, %2\n"
        : "=&r"(old), "=&r"(mask)
        : "i"(csr_num), "r"(mask)
    );
    barrier_inst();
    return old;
}
```

#### 3.3 内存屏障操作

```c
/**
 * @brief 数据同步屏障（完成屏障）
 * @note 确保所有内存操作完成
 */
#define barrier() \
    __asm__ volatile("dbar 0" ::: "memory")

/**
 * @brief 数据排序屏障（加载）
 * @note 确保之前的加载操作完成
 */
#define barrier_load() \
    __asm__ volatile("dbar 0b10101" ::: "memory")

/**
 * @brief 数据排序屏障（存储）
 * @note 确保之前的存储操作完成
 */
#define barrier_store() \
    __asm__ volatile("dbar 0b11010" ::: "memory")

/**
 * @brief 指令同步屏障
 * @note 确保之前的指令完成
 */
#define barrier_inst() \
    __asm__ volatile("ibar 0")

/**
 * @brief 完整屏障（数据 + 指令）
 * @note 确保所有内存操作和指令完成
 */
#define full_barrier() \
    do { \
        __asm__ volatile("dbar 0" ::: "memory"); \
        __asm__ volatile("ibar 0"); \
    } while (0)
```

#### 3.4 高级内联汇编模式

**LL/SC（Load-Linked/Store-Conditional）模式**：

```c
/**
 * @brief 原子比较交换（CAS）操作
 * @param ptr 目标指针
 * @param expected 期望值
 * @param desired 新值
 * @return 成功返回true，失败返回false
 */
static inline bool atomic_cas_llsc(uint32_t *ptr,
                                   uint32_t expected,
                                   uint32_t desired)
{
    uint32_t tmp;
    int result;

    __asm__ volatile(
        "1: \n"
        "   ll.w  %0, %2  \n"     /* 加载链接 */
        "   li.w  %1, 1     \n"
        "   beq   %0, %3, 2f \n"  /* 比较是否相等 */
        "   sc.w  %1, %4  \n"     /* 条件存储 */
        "   beq   $zero, %1, 2f \n"  /* 检查是否成功 */
        "   b     1b        \n"     /* 重试 */
        "2: \n"
        : "=&r"(tmp), "=&r"(result)
        : "m"(*ptr), "r"(expected), "r"(desired)
        : "memory"
    );

    return (result == 0);
}
```

### 4. 原子操作规范

#### 4.1 C11原子操作

```c
#include <stdatomic.h>

/**
 * @brief 原子加载（获取语义）
 */
static inline uint32_t atomic_load_acquire(atomic_uint *ptr)
{
    return atomic_load_explicit(ptr, memory_order_acquire);
}

/**
 * @brief 原子存储（释放语义）
 */
static inline void atomic_store_release(atomic_uint *ptr, uint32_t value)
{
    atomic_store_explicit(ptr, value, memory_order_release);
}

/**
 * @brief 原子加法（返回旧值）
 */
static inline uint32_t atomic_fetch_add(atomic_uint *ptr, uint32_t value)
{
    return atomic_fetch_add_explicit(ptr, value, memory_order_acq_rel);
}

/**
 * @brief 原子比较交换
 */
static inline bool atomic_cas(atomic_uint *ptr,
                               uint32_t *expected,
                               uint32_t desired)
{
    return atomic_compare_exchange_strong_explicit(
        ptr, expected, desired,
        memory_order_acq_rel,
        memory_order_acquire
    );
}
```

#### 4.2 LoongArch64特定原子操作

```c
/**
 * @brief 原自增操作
 */
static inline uint32_t atomic_inc(volatile uint32_t *ptr)
{
    uint32_t old;
    uint32_t new_val;

    do
    {
        old = *ptr;
        new_val = old + 1U;
        __asm__ volatile("" ::: "memory");  /* 编译器屏障 */
    }
    while (!__builtin_expect(
        __sync_bool_compare_and_swap(ptr, old, new_val), 1
    ));

    return old;
}

/**
 * @brief 原子减操作
 */
static inline uint32_t atomic_dec(volatile uint32_t *ptr)
{
    uint32_t old;
    uint32_t new_val;

    do
    {
        old = *ptr;
        new_val = old - 1U;
        __asm__ volatile("" ::: "memory");
    }
    while (!__builtin_expect(
        __sync_bool_compare_and_swap(ptr, old, new_val), 1
    ));

    return old;
}
```

### 5. 多核SMP编程规范

#### 5.1 自旋锁模式

**Ticket Lock（公平自旋锁）**：

```c
/**
 * @brief Ticket锁结构体
 */
typedef struct
{
    atomic_uint_fast16_t next_ticket;
    atomic_uint_fast16_t serving_ticket;
} TicketLock_t;

/**
 * @brief 初始化Ticket锁
 */
#define TICKET_LOCK_INIT {0, 0}

/**
 * @brief 获取Ticket锁
 */
static inline void ticket_lock_acquire(TicketLock_t *lock)
{
    uint16_t my_ticket = (uint16_t)atomic_fetch_add(&lock->next_ticket, 1U);

    while (atomic_load(&lock->serving_ticket) != my_ticket)
    {
        /* LoongArch64降低功耗：使用idle指令 */
        __asm__ volatile("idle 0");
    }

    /* 获取锁后的内存屏障 */
    barrier();
}

/**
 * @brief 释放Ticket锁
 */
static inline void ticket_lock_release(TicketLock_t *lock)
{
    /* 释放锁前的内存屏障 */
    barrier();
    atomic_fetch_add(&lock->serving_ticket, 1U);
}
```

#### 5.2 核心间中断（IPI）

**IPI类型定义**：

```c
#define IPI_RESCHEDULE   0U  /* 重新调度 */
#define IPI_STOP         1U  /* 停止CPU */
#define IPI_TIMER        2U  /* 定时器中断 */
#define IPI_CALL_FUNC    3U  /* 调用函数 */
#define IPI_CPU_BACKTRACE 4U /* CPU回溯 */

/**
 * @brief 发送IPI到指定CPU
 * @param target_cpu 目标CPU编号
 * @param ipi_type IPI类型
 */
static inline void ipi_send(uint32_t target_cpu, uint32_t ipi_type)
{
    uint32_t cpu_mask = (1U << target_cpu);

    /* 写入LoongArch IPI寄存器 */
    uint64_t ipi_reg = ((uint64_t)cpu_mask << 16U) |
                       ((uint64_t)ipi_type);

    __asm__ volatile(
        "csrwr %0, 0x1048"  /* LOONGARCH_IOCSR_IPI_SEND */
        :: "r"(ipi_reg)
    );
}

/**
 * @brief 发送IPI到所有CPU（除了自己）
 * @param ipi_type IPI类型
 */
static inline void ipi_broadcast(uint32_t ipi_type)
{
    uint64_t ipi_reg = ((uint64_t)0xFFU << 16U) |  /* 所有CPU */
                       ((uint64_t)ipi_type);

    __asm__ volatile(
        "csrwr %0, 0x1048"
        :: "r"(ipi_reg)
    );
}
```

#### 5.3 禁止抢占

```c
/**
 * @brief 禁止调度器（禁止任务切换）
 */
static inline void scheduler_disable(void)
{
    uint32_t cpu_id = get_cpu_id();
    scheduler.lock_count[cpu_id]++;
    barrier();
}

/**
 * @brief 使能调度器
 */
static inline void scheduler_enable(void)
{
    uint32_t cpu_id = get_cpu_id();

    scheduler.lock_count[cpu_id]--;
    barrier();

    if (scheduler.lock_count[cpu_id] == 0U)
    {
        schedule();  /* 触发调度 */
    }
}
```

### 6. 异常和中断处理规范

#### 6.1 特权级

**LoongArch64特权级（Priority Level, PLV）**：

```c
#define PLV0    0U   /* 内核模式（Kernel mode） */
#define PLV1    1U   /* 保留 */
#define PLV2    2U   /* 保留 */
#define PLV3    3U   /* 用户模式（User mode） */

/**
 * @brief 获取当前特权级
 * @return 当前特权级（PLV0-PLV3）
 */
static inline uint32_t get_current_plv(void)
{
    uint32_t crmd;
    __asm__ volatile("csrrd %0, 0x0" : "=r"(crmd));  /* LOONGARCH_CSR_CRMD */
    return (crmd & 0x3U);  /* 提取PLV位 */
}

/**
 * @brief 从内核模式切换到用户模式
 */
static inline void drop_to_plv3(void)
{
    uint32_t prmd;
    __asm__ volatile("csrrd %0, 0x1" : "=r"(prmd));  /* LOONGARCH_CSR_PRMD */
    prmd = (prmd & ~0x3U) | PLV3;
    __asm__ volatile("csrwr %0, 0x1" :: "r"(prmd));  /* LOONGARCH_CSR_PRMD */
    __asm__ volatile("ertn");
}

/**
 * @brief 进入内核模式
 */
static inline void enter_kernel_mode(void)
{
    /* 保存当前PLV */
    uint32_t prmd = read_csr(0x1);  /* LOONGARCH_CSR_PRMD */

    /* 设置PLV0 */
    prmd &= ~0x3U;
    write_csr(0x1, prmd);
}
```

#### 6.2 异常向量表

**异常码定义**：

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

**异常向量表结构**：

```c
/**
 * @brief 异常处理函数类型
 */
typedef void (*ExceptionHandler_t)(void);

/**
 * @brief 异常向量表
 * @note 必须对齐到VECSIZE（通常为512字节）
 */
__attribute__((aligned(512))) ExceptionHandler_t exception_table[64] =
{
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
    [EXCCODE_SYS]       = handle_sys,          /* 系统调用 */
    [EXCCODE_BP]        = handle_bp,           /* 断点 */
    [EXCCODE_FPDIS]     = handle_fpu,          /* FPU禁用 */

    /* 浮点和SIMD */
    [EXCCODE_LSXDIS]    = handle_lsx,          /* LSX禁用 */
    [EXCCODE_LASXDIS]   = handle_lasx,         /* LASX禁用 */
    [EXCCODE_FPE]       = handle_fpe,          /* 浮点异常 */

    /* 中断向量 */
    [EXCCODE_INT_START ... EXCCODE_INT_END] = handle_vint,
};
```

### 7. MMU和TLB管理规范

#### 7.1 MMU操作

**页表操作**：

```c
/**
 * @brief 设置页表基地址
 * @param pgdl 页表基地址（低地址）
 */
static inline void set_page_table_base(uint64_t pgdl)
{
    write_csr(0x19, pgdl);  /* LOONGARCH_CSR_PGDL */
    full_barrier();
}

/**
 * @brief 获取页表基地址
 * @return 页表基地址（低地址）
 */
static inline uint64_t get_page_table_base(void)
{
    return read_csr(0x19);  /* LOONGARCH_CSR_PGDL */
}

/**
 * @brief 使能MMU
 */
static inline void mmu_enable(void)
{
    uint32_t crmd = read_csr(0x0);  /* LOONGARCH_CSR_CRMD */
    crmd |= (1U << 4);  /* 设置DA位 */
    write_csr(0x0, crmd);
    barrier();
}

/**
 * @brief 禁用MMU
 */
static inline void mmu_disable(void)
{
    uint32_t crmd = read_csr(0x0);  /* LOONGARCH_CSR_CRMD */
    crmd &= ~(1U << 4);  /* 清除DA位 */
    write_csr(0x0, crmd);
    barrier();
}
```

**页表项格式**：

```c
/**
 * @brief 页表项标志位
 */
#define PTE_V       (1UL << 0)  /* 有效位 */
#define PTE_R       (1UL << 1)  /* 可读 */
#define PTE_W       (1UL << 2)  /* 可写 */
#define PTE_X       (1UL << 3)  /* 可执行 */
#define PTE_U       (1UL << 4)  /* 用户可访问 */
#define PTE_G       (1UL << 5)  /* 全局页 */
#define PTE_A       (1UL << 6)  /* 已访问 */
#define PTE_D       (1UL << 7)  /* 已修改 */
#define PTE_PLV0    (0UL << 8)  /* PLV0可访问 */
#define PTE_PLV1    (1UL << 8)  /* PLV1可访问 */
#define PTE_PLV2    (2UL << 8)  /* PLV2可访问 */
#define PTE_PLV3    (3UL << 8)  /* PLV3可访问 */

/**
 * @brief 创建页表项
 * @param phys_addr 物理地址
 * @param flags 标志位
 * @return 页表项值
 */
static inline uint64_t pte_create(uint64_t phys_addr, uint64_t flags)
{
    return (phys_addr & ~0xFFFUL) | flags;
}
```

#### 7.2 TLB操作

**TLB维护操作**：

```c
/**
 * @brief 使TLB项无效（所有地址）
 */
static inline void tlb_invalidate_all(void)
{
    __asm__ volatile("invtlb 0x0, 0x0, 0x0");  /* INVTLB_ALL */
    full_barrier();
}

/**
 * @brief 使TLB项无效（指定地址）
 * @param addr 虚拟地址
 */
static inline void tlb_invalidate_page(uint64_t addr)
{
    uint64_t page = addr >> 12U;
    __asm__ volatile("invtlb 0x24, %0, %1" :: "r"(page), "r"(0));  /* INVTLB_ADDR */
    full_barrier();
}

/**
 * @brief 使TLB项无效（指定ASID和地址）
 * @param asid 地址空间ID
 * @param addr 虚拟地址
 */
static inline void tlb_invalidate_asid(uint32_t asid, uint64_t addr)
{
    uint64_t page = addr >> 12U;
    uint64_t val = ((uint64_t)asid << 48U) | page;
    __asm__ volatile("invtlb 0x20, %0, %1" :: "r"(val), "r"(0));  /* INVTLB_ASID */
    full_barrier();
}

/**
 * @brief 读取TLB项
 * @param index TLB索引
 * @param[out] entryhi TLB EntryHi值
 * @param[out] entrylo0 TLB EntryLo0值
 * @param[out] entrylo1 TLB EntryLo1值
 */
static inline void tlb_read(uint32_t index,
                             uint64_t *entryhi,
                             uint64_t *entrylo0,
                             uint64_t *entrylo1)
{
    __asm__ volatile(
        "tlbsrch %0\n"
        "tlbrd\n"
        :: "r"(index)
    );

    *entryhi = read_csr(0x1A);  /* TLB_EHI */
    *entrylo0 = read_csr(0x1B); /* TLB_ELO0 */
    *entrylo1 = read_csr(0x1C); /* TLB_ELO1 */
}

/**
 * @brief 写入TLB项
 * @param index TLB索引
 * @param entryhi TLB EntryHi值
 * @param entrylo0 TLB EntryLo0值
 * @param entrylo1 TLB EntryLo1值
 */
static inline void tlb_write(uint32_t index,
                             uint64_t entryhi,
                             uint64_t entrylo0,
                             uint64_t entrylo1)
{
    write_csr(0x1A, entryhi);  /* TLB_EHI */
    write_csr(0x1B, entrylo0); /* TLB_ELO0 */
    write_csr(0x1C, entrylo1); /* TLB_ELO1 */

    __asm__ volatile(
        "tlbwr %0\n"
        :: "r"(index)
    );

    full_barrier();
}

/**
 * @brief 查找TLB项
 * @param vaddr 虚拟地址
 * @param asid 地址空间ID
 * @return TLB索引（未找到返回-1）
 */
static inline int32_t tlb_probe(uint64_t vaddr, uint32_t asid)
{
    uint64_t val = ((uint64_t)asid << 48U) | (vaddr >> 12U);

    __asm__ volatile(
        "tlbsrch %0\n"
        :: "r"(val)
    );

    /* 读取TLB索引寄存器 */
    uint32_t index = (uint32_t)read_csr(0x1D); /* TLB_IDX */

    if (index & 0x80000000U)
    {
        return -1;  /* 未找到 */
    }

    return (int32_t)index;
}
```

### 8. 缓存操作规范

#### 8.1 缓存维护操作

```c
/**
 * @brief 清理数据缓存到内存
 * @param addr 起始地址
 * @param size 大小（字节）
 */
static inline void dcache_clean(void *addr, uint32_t size)
{
    uint64_t start = (uint64_t)addr;
    uint64_t end = start + (uint64_t)size;

    start &= ~0x3FULL;  /* 64字节缓存行对齐 */

    while (start < end)
    {
        __asm__ volatile(
            "dbar 0\n"
            "cache 0x8, %0"  /* D-Cache Clean */
            :: "r"(start)
        );
        start += 64U;
    }

    barrier();
}

/**
 * @brief 使数据缓存无效
 * @param addr 起始地址
 * @param size 大小（字节）
 */
static inline void dcache_invalidate(void *addr, uint32_t size)
{
    uint64_t start = (uint64_t)addr;
    uint64_t end = start + (uint64_t)size;

    start &= ~0x3FULL;

    while (start < end)
    {
        __asm__ volatile(
            "dbar 0\n"
            "cache 0x9, %0"  /* D-Cache Invalidate */
            :: "r"(start)
        );
        start += 64U;
    }

    barrier();
}

/**
 * @brief 清理并使数据缓存无效
 * @param addr 起始地址
 * @param size 大小（字节）
 */
static inline void dcache_clean_and_invalidate(void *addr, uint32_t size)
{
    uint64_t start = (uint64_t)addr;
    uint64_t end = start + (uint64_t)size;

    start &= ~0x3FULL;

    while (start < end)
    {
        __asm__ volatile(
            "dbar 0\n"
            "cache 0x18, %0"  /* D-Cache Clean and Invalidate */
            :: "r"(start)
        );
        start += 64U;
    }

    barrier();
}

/**
 * @brief 使指令缓存无效
 * @param addr 起始地址
 * @param size 大小（字节）
 */
static inline void icache_invalidate(void *addr, uint32_t size)
{
    uint64_t start = (uint64_t)addr;
    uint64_t end = start + (uint64_t)size;

    start &= ~0x3FULL;

    while (start < end)
    {
        __asm__ volatile(
            "ibar 0\n"
            "cache 0x1, %0"  /* I-Cache Invalidate */
            :: "r"(start)
        );
        start += 64U;
    }

    barrier_inst();
}
```

#### 8.2 缓存同步操作

```c
/**
 * @brief 同步缓存到内存（多核数据共享）
 * @param addr 共享数据地址
 * @param size 数据大小
 * @param target_cpu 目标CPU编号
 *
 * @note 在发送IPI前调用此函数确保数据已写入内存
 */
static inline void cache_sync_to_core(void *addr, uint32_t size, uint32_t target_cpu)
{
    /* 清理数据缓存到内存 */
    dcache_clean(addr, size);

    /* 确保数据已到达一致性点 */
    full_barrier();

    /* 向目标核发送IPI */
    ipi_send(target_cpu, IPI_DATA_READY);
}
```

### 9. 时间和定时器操作

#### 9.1 读取时间戳

```c
/**
 * @brief 读取稳定时间戳（64位）
 * @return 时间戳计数
 */
static inline uint64_t read_time(void)
{
    uint64_t count;
    __asm__ volatile("rdtime.d %0, $zero" : "=r"(count));
    return count;
}

/**
 * @brief 读取稳定时间戳（32位低半部分）
 * @return 时间戳计数（低32位）
 */
static inline uint32_t read_time_l(void)
{
    uint32_t count;
    __asm__ volatile("rdtime.w %0, $zero" : "=r"(count));
    return count;
}

/**
 * @brief 读取稳定时间戳频率
 * @return 时间戳频率（Hz）
 */
static inline uint64_t read_time_freq(void)
{
    uint64_t freq;
    __asm__ volatile("rdtime.d %0, $zero" : "=r"(freq));
    return freq;
}
```

#### 9.2 定时器操作

```  c
/**
 * @brief 设置定时器
 * @param value 定时器值
 */
static inline void set_timer(uint64_t value)
{
    __asm__ volatile("csrwr %0, 0x22" :: "r"(value));  /* LOONGARCH_CSR_TCFG */
}

/**
 * @brief 读取定时器
 * @return 定时器值
 */
static inline uint64_t read_timer(void)
{
    return read_csr(0x22);  /* LOONGARCH_CSR_TCFG */
}
```

---

## 总结

### 必须遵循的原则

1. **数据类型**：使用固定宽度整数类型（int32_t、uint64_t等）
2. **对齐要求**：栈16字节对齐，缓存行64字节对齐，页4KB对齐
3. **内联汇编**：使用volatile关键字，正确指定约束和clobber列表
4. **原子操作**：多核共享数据必须使用原子操作或锁保护
5. **内存屏障**：正确使用dbar/ibar指令确保内存一致性
6. **异常处理**：正确设置异常向量表，遵循特权级规则
7. **MMU/TLB**：正确维护页表和TLB，确保地址空间隔离
8. **缓存一致性**：多核数据共享时正确维护缓存一致性

### 禁止模式

- ❌ 禁止使用隐式类型转换
- ❌ 禁止在多核环境访问共享数据不加保护
- ❌ 禁止忘记添加内存屏障
- ❌ 禁止使用未初始化的页表项
- ❌ 禁止忘记清理缓存后发送IPI
- ❌ 禁止在内联汇编中缺少volatile关键字

---

**记住**：LoongArch64是弱内存模型架构，必须显式使用内存屏障确保正确的内存访问顺序。
