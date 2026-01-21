# 核心原则

## 1. 安全关键编码原则

### 1.1 可预测性优先

所有代码行为必须**可预测**且**确定性**。严格禁止未定义行为（UB）。

```c
/* ❌ 错误：未定义行为 - 有符号整数溢出 */
int32_t a = INT32_MAX;
int32_t b = a + 1;  /* UB - 有符号溢出回绕 */

/* ✅ 正确：检查溢出 */
int32_t a = INT32_MAX;
int32_t b;
if (a < INT32_MAX) {
    b = a + 1;
}
```

### 1.2 显式优于隐式

每个类型转换、操作和假设都必须**显式声明**。

```c
/* ❌ 错误：隐式类型转换 */
uint32_t x = 10;
int32_t y = -5;
if (x > y) {  /* y被隐式转换为uint32_t */
}

/* ✅ 正确：显式比较 */
uint32_t x = 10U;
int32_t y = -5;
if ((y >= 0) && (x > (uint32_t)y)) {
}
```

### 1.3 最小权限原则

使用 `const` 和 `volatile` 限定符限制数据访问范围。

```c
/* ✅ 正确：对只读数据使用const */
void print_string(const char *str) {
    /* str不能被修改 */
}

/* ✅ 正确：对硬件寄存器使用volatile */
#define GPIO_BASE ((volatile uint32_t *)0x3F200000U)
```

### 1.4 防御性编程

假设所有外部输入（参数、硬件、网络）都可能**有问题或错误**。

```c
/* ✅ 正确：验证所有参数 */
ErrorCode_t buffer_write(uint8_t *buf, uint32_t size, uint8_t data) {
    if (buf == NULL) {
        return ERROR_INVALID_PARAM;
    }
    if (size == 0U) {
        return ERROR_INVALID_PARAM;
    }
    /* ... */
}
```

### 1.5 内存安全

确保没有内存泄漏、越界访问或悬空指针。

```c
/* ❌ 错误：内存泄漏 */
void function(void) {
    void *ptr = malloc(100);
    return;  /* 忘记释放ptr */
}

/* ✅ 正确：总是释放已分配的内存 */
void function(void) {
    void *ptr = malloc(100);
    if (ptr != NULL) {
        /* 使用ptr */
        free(ptr);
        ptr = NULL;  /* 防止悬空指针 */
    }
}
```

---

## 2. 多核SMP安全原则

### 2.1 原子操作

核间共享的所有数据必须使用**原子操作**或**锁保护**。

```c
/* ✅ 正确：对共享计数器使用原子操作 */
#include <stdatomic.h>
atomic_uint g_task_counter;

void increment_task_counter(void) {
    atomic_fetch_add(&g_task_counter, 1U);
}

/* ❌ 错误：共享数据未加保护 */
uint32_t g_shared_counter = 0U;  /* 多核访问不安全！ */

void increment_counter(void) {
    g_shared_counter++;  /* 竞争条件！ */
}
```

### 2.2 内存屏障

LoongArch使用**弱内存模型**。始终使用正确的内存屏障。

#### 2.2.1 内存屏障类型

```c
/* 完整屏障：确保所有内存操作完成 */
barrier();

/* 加载屏障：确保之前的加载操作完成 */
barrier_load();

/* 存储屏障：确保之前的存储操作完成 */
barrier_store();

/* 指令屏障：确保之前的指令完成 */
barrier_inst();
```

#### 2.2.2 发布-获取模式

```c
/* ✅ 正确：使用发布-获取语义 */
typedef struct {
    atomic_uintptr_t data_ptr;
    atomic_uint ready;
} SharedData_t;

/* 生产者：发布数据 */
void producer_publish(SharedData_t *shared, void *data) {
    /* 1. 写入数据 */
    prepare_data(data);

    /* 2. 存储屏障：确保数据先写入 */
    barrier_store();

    /* 3. 设置就绪标志（release语义） */
    atomic_store_explicit(&shared->ready, 1U, memory_order_release);
}

/* 消费者：获取数据 */
void *consumer_acquire(SharedData_t *shared) {
    /* 1. 检查就绪标志（acquire语义） */
    if (atomic_load_explicit(&shared->ready, memory_order_acquire) == 0U)
    {
        return NULL;
    }

    /* 2. 加载屏障：确保数据先读取 */
    barrier_load();

    /* 3. 读取数据 */
    return (void *)atomic_load_explicit(&shared->data_ptr, memory_order_relaxed);
}
```

### 2.3 缓存一致性

使用缓存维护操作确保多核数据一致性。

#### 2.3.1 缓存同步操作

```c
/* ✅ 正确：在共享数据前清理缓存 */
void share_buffer_to_core(uint8_t *buf, uint32_t size, uint32_t target_core) {
    /* 1. 写入数据 */
    memset(buf, 0xAA, size);

    /* 2. 清理数据缓存到一致性点 */
    dcache_clean(buf, size);

    /* 3. 确保缓存操作完成 */
    full_barrier();

    /* 4. 向目标核发送IPI */
    ipi_send(target_core, IPI_DATA_READY);
}

/* ✅ 正确：接收核使缓存无效 */
void receive_shared_buffer(uint8_t *buf, uint32_t size) {
    /* 1. 使数据缓存无效 */
    dcache_invalidate(buf, size);

    /* 2. 确保缓存操作完成 */
    full_barrier();

    /* 3. 现在可以安全读取数据 */
    process_data(buf, size);
}
```

#### 2.3.2 缓存行对齐

```c
/* ✅ 正确：缓存行对齐避免伪共享 */
typedef struct __attribute__((aligned(64))) {
    atomic_uint64_t lock;
    uint32_t task_count;
    uint8_t padding[64 - sizeof(atomic_uint64_t) - sizeof(uint32_t)];
} PerCPUData_t;

/* 每CPU数据独立 */
PerCPUData_t per_cpu_data[MAX_CPUS];
```

### 2.4 无锁编程

在高争用场景下优先使用无锁数据结构。

#### 2.4.1 SPSC队列（单生产者单消费者）

```c
/* ✅ 正确：无锁SPSC队列 */
typedef struct {
    uint32_t buffer[256];
    atomic_uint head;
    atomic_uint tail;
} SPSCQueue_t;

void spsc_enqueue(SPSCQueue_t *queue, uint32_t value) {
    uint32_t index = atomic_load_explicit(&queue->head, memory_order_relaxed);
    uint32_t next = (index + 1U) & 0xFFU;

    /* 检查队列是否满 */
    if (next == atomic_load_explicit(&queue->tail, memory_order_acquire))
    {
        return;  /* 队列满 */
    }

    /* 写入数据 */
    queue->buffer[index] = value;

    /* 存储屏障 */
    barrier_store();

    /* 更新head指针（release语义） */
    atomic_store_explicit(&queue->head, next, memory_order_release);
}

bool spsc_dequeue(SPSCQueue_t *queue, uint32_t *value) {
    uint32_t index = atomic_load_explicit(&queue->tail, memory_order_relaxed);

    /* 检查队列是否空 */
    if (index == atomic_load_explicit(&queue->head, memory_order_acquire))
    {
        return false;  /* 队列空 */
    }

    /* 读取数据 */
    *value = queue->buffer[index];

    /* 加载屏障 */
    barrier_load();

    /* 更新tail指针（release语义） */
    atomic_store_explicit(&queue->tail, (index + 1U) & 0xFFU, memory_order_release);

    return true;
}
```

#### 2.4.2 无锁栈（ABA问题）

```c
/* ✅ 正确：使用版本号解决ABA问题 */
typedef struct StackNode {
    uint32_t value;
    struct StackNode *next;
} StackNode_t;

typedef struct {
    atomic_uintptr_t top;  /* 格式: [版本号:32位][指针:32位] */
} LockFreeStack_t;

void stack_push(LockFreeStack_t *stack, uint32_t value) {
    StackNode_t *node = malloc(sizeof(StackNode_t));
    if (node == NULL) {
        return;
    }

    node->value = value;

    uint64_t old_top;
    uint64_t new_top;

    do {
        old_top = atomic_load(&stack->top);
        node->next = (StackNode_t *)(uintptr_t)(old_top & 0xFFFFFFFFUL);
        new_top = ((old_top + 0x100000000ULL) & ~0xFFFFFFFFUL) |
                  (uintptr_t)node;
    } while (!atomic_compare_exchange_weak(&stack->top, &old_top, new_top));
}
```

### 2.5 核心间中断（IPI）

#### 2.5.1 IPI类型和处理

```c
/* IPI类型定义 */
#define IPI_RESCHEDULE   0U  /* 重新调度 */
#define IPI_STOP         1U  /* 停止CPU */
#define IPI_TIMER        2U  /* 定时器中断 */
#define IPI_CALL_FUNC    3U  /* 调用函数 */
#define IPI_TLB_FLUSH    4U  /* TLB刷新 */

/**
 * @brief IPI处理函数
 */
void ipi_handler(void) {
    uint32_t ipi_type = read_ipi_type();

    switch (ipi_type)
    {
        case IPI_RESCHEDULE:
            /* 设置调度标志 */
            set_need_reschedule();
            break;

        case IPI_STOP:
            /* 停止当前CPU */
            cpu_stop();
            break;

        case IPI_TLB_FLUSH:
            /* 刷新TLB */
            tlb_invalidate_all();
            break;

        case IPI_CALL_FUNC:
            /* 执行跨CPU函数调用 */
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

#### 2.5.2 IPI同步机制

```c
/**
 * @brief 向所有CPU发送IPI并等待完成
 * @param ipi_type IPI类型
 * @return 成功返回0，失败返回错误码
 */
int ipi_broadcast_sync(uint32_t ipi_type) {
    uint32_t cpu_count = get_cpu_count();
    atomic_uint completion_count;

    atomic_init(&completion_count, 0);

    /* 向所有CPU（除了自己）发送IPI */
    for (uint32_t cpu = 0U; cpu < cpu_count; cpu++)
    {
        if (cpu != get_current_cpu())
        {
            set_ipi_callback(cpu, &ipi_completion_callback, &completion_count);
            ipi_send(cpu, ipi_type);
        }
    }

    /* 等待所有CPU完成 */
    uint32_t timeout = 1000;  /* 1秒超时 */
    while (atomic_load(&completion_count) < (cpu_count - 1U))
    {
        if (timeout-- == 0U)
        {
            return -ETIMEDOUT;  /* 超时 */
        }
        udelay(1000);  /* 延迟1ms */
    }

    return 0;
}
```

### 2.6 负载均衡

#### 2.6.1 任务迁移

```c
/**
 * @brief 任务迁移决策
 * @param task 要迁移的任务
 * @return 目标CPU编号，迁移失败返回-1
 */
int scheduler_migrate_task(Task_t *task) {
    uint32_t current_cpu = task->cpu;
    uint32_t target_cpu = find_least_loaded_cpu();

    /* 如果当前CPU已经是负载最低的，不迁移 */
    if (target_cpu == current_cpu) {
        return -1;
    }

    /* 锁定两个CPU的运行队列 */
    spin_lock(&cpu_rq_lock(current_cpu));
    spin_lock(&cpu_rq_lock(target_cpu));

    /* 从当前CPU移除任务 */
    remove_task_from_rq(current_cpu, task);

    /* 添加到目标CPU */
    add_task_to_rq(target_cpu, task);

    /* 更新任务CPU亲和性 */
    task->cpu = target_cpu;

    /* 释放锁 */
    spin_unlock(&cpu_rq_lock(target_cpu));
    spin_unlock(&cpu_rq_lock(current_cpu));

    /* 向目标CPU发送IPI */
    ipi_send(target_cpu, IPI_RESCHEDULE);

    return (int)target_cpu;
}
```

#### 2.6.2 负载计算

```c
/**
 * @brief 计算CPU负载
 * @param cpu CPU编号
 * @return 负载值（0-1000）
 */
uint32_t cpu_load(uint32_t cpu) {
    RunQueue_t *rq = &cpu_runqueues[cpu];
    uint32_t runnable = rq->nr_running;
    uint32_t load = rq->load.weight;

    /* 考虑可运行任务数量和负载权重 */
    return (runnable * 1000U) / (runnable + load);
}
```

### 2.7 禁止抢占

```c
/**
 * @brief 禁止调度器（禁止任务切换）
 */
static inline void scheduler_disable(void) {
    uint32_t cpu_id = get_cpu_id();
    scheduler.lock_count[cpu_id]++;
    barrier();
}

/**
 * @brief 使能调度器
 */
static inline void scheduler_enable(void) {
    uint32_t cpu_id = get_cpu_id();

    scheduler.lock_count[cpu_id]--;
    barrier();

    if (scheduler.lock_count[cpu_id] == 0U) {
        schedule();  /* 触发调度 */
    }
}

/* 使用示例 */
void critical_section(void) {
    scheduler_disable();  /* 禁止调度 */

    /* 临界区代码（不会被抢占） */
    update_shared_data();

    scheduler_enable();  /* 恢复调度 */
}
```

### 2.8 多核死锁预防

#### 2.8.1 锁顺序规范

```c
/* 定义全局锁顺序 */
typedef enum {
    LOCK_ORDER_SCHEDULER = 0,
    LOCK_ORDER_MEMORY,
    LOCK_ORDER_TASK,
    LOCK_ORDER_IPI,
    LOCK_ORDER_MAX
} LockOrder_t;

/* ✅ 正确：按照相同顺序获取锁 */
void multi_lock_function(void) {
    /* 先获取scheduler锁 */
    spin_lock(&scheduler.lock);

    /* 再获取memory锁 */
    spin_lock(&memory.lock);

    /* 临界区代码 */

    /* 按相反顺序释放 */
    spin_unlock(&memory.lock);
    spin_unlock(&scheduler.lock);
}

/* ❌ 错误：不按顺序获取锁 */
void deadlock_prone_function(void) {
    spin_lock(&memory.lock);  /* 与上面的顺序相反！ */
    spin_lock(&scheduler.lock);

    /* 可能死锁！ */
}
```

#### 2.8.2 超时机制

```c
/**
 * @brief 带超时的自旋锁获取
 * @param lock 自旋锁指针
 * @param timeout_us 超时时间（微秒）
 * @return 成功返回true，超时返回false
 */
bool spin_lock_timeout(Spinlock_t *lock, uint32_t timeout_us) {
    uint64_t start = get_time_us();

    while (!spin_try_lock(lock)) {
        if ((get_time_us() - start) >= timeout_us) {
            return false;  /* 超时 */
        }
        __asm__ volatile("idle 0");  /* 降低功耗 */
    }

    return true;
}
```

### 2.9 核心间数据共享

#### 2.9.1 Per-CPU变量

```c
/* ✅ 正确：使用Per-CPU变量避免竞争 */
#define DEFINE_PER_CPU(type, name) \
    type name[MAX_CPUS] __attribute__((aligned(64)))

DEFINE_PER_CPU(uint32_t, task_count);  /* 每CPU任务计数 */

void increment_task_count(void) {
    uint32_t cpu = get_cpu_id();
    per_cpu_task_count[cpu]++;  /* 无需加锁 */
}

uint32_t get_task_count(void) {
    uint32_t total = 0U;
    for (uint32_t cpu = 0U; cpu < MAX_CPUS; cpu++) {
        total += per_cpu_task_count[cpu];
    }
    return total;
}
```

#### 2.9.2 读写锁

```c
/* ✅ 正确：读写锁适合读多写少场景 */
typedef struct {
    atomic_uint readers;
    atomic_uint writers;
    Spinlock_t lock;
} RWLock_t;

void rwlock_read_lock(RWLock_t *rw) {
    /* 等待写者完成 */
    while (atomic_load(&rw->writers) > 0U) {
        __asm__ volatile("idle 0");
    }

    /* 增加读者计数 */
    atomic_fetch_add(&rw->readers, 1U);
    barrier();
}

void rwlock_read_unlock(RWLock_t *rw) {
    atomic_fetch_sub(&rw->readers, 1U);
}

void rwlock_write_lock(RWLock_t *rw) {
    spin_lock(&rw->lock);

    /* 等待所有读者完成 */
    while (atomic_load(&rw->readers) > 0U) {
        __asm__ volatile("idle 0");
    }

    /* 设置写者标志 */
    atomic_store(&rw->writers, 1U);
    barrier();
}

void rwlock_write_unlock(RWLock_t *rw) {
    atomic_store(&rw->writers, 0U);
    spin_unlock(&rw->lock);
}

---

## 3. 功能安全原则

### 3.1 可追溯性

每个代码模块必须对应**需求文档**。

```c
/**
 * @file scheduler.c
 * @brief 256级优先级调度器实现
 *
 * 需求: SCH-001 - 系统应支持256个优先级
 * 需求: SCH-002 - 调度器应使用O(1)选择算法
 * 需求: SCH-003 - 支持最多8个CPU核心，带负载均衡
 */
```

### 3.2 可测试性

所有代码必须**可单元测试**，MC/DC覆盖率>95%。

```c
/* ✅ 正确：为可测试性设计 */
typedef struct {
    int32_t (*read_func)(void);  /* 注入依赖 */
} Timer_t;

int32_t timer_get_ticks(Timer_t *timer) {
    return timer->read_func();  /* 可在测试中模拟 */
}
```

### 3.3 可验证性

限制复杂度以支持形式化验证（最大圈复杂度：10）。

```c
/* ❌ 错误：太复杂（复杂度 = 15） */
void complex_function(uint32_t x, uint32_t y, uint32_t z) {
    if (x > 0) {
        if (y > 0) {
            if (z > 0) {
                /* ... 深度嵌套 ... */
            }
        }
    }
}

/* ✅ 正确：重构为更小的函数 */
void complex_function(uint32_t x, uint32_t y, uint32_t z) {
    if (x > 0U) {
        handle_positive_x(y, z);
    } else {
        handle_zero_x(y, z);
    }
}
```

### 3.4 错误处理

**所有可能的错误路径**都必须显式处理。

```c
/* ✅ 正确：处理每种错误情况 */
ErrorCode_t page_alloc(uint64_t *addr) {
    if (addr == NULL) {
        return ERROR_INVALID_PARAM;
    }

    void *page = malloc(PAGE_SIZE);
    if (page == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    if ((uintptr_t)page & (PAGE_SIZE - 1U)) {
        free(page);
        return ERROR_ALIGNMENT;
    }

    *addr = (uint64_t)page;
    return ERROR_SUCCESS;
}
```

---

## 4. 安全生命周期合规

### 4.1 IEC 61508 (SIL 4)

- **验证**：所有代码由至少2名独立审查者审查
- **确认**：所有需求追溯到测试用例
- **配置管理**：所有更改在版本控制中跟踪
- **危害分析**：识别并缓解所有潜在故障模式

### 4.2 ISO 26262 (ASIL D)

- **ASIL分解**：对关键功能使用冗余多样化实现
- **免于干扰**：分区安全关键组件
- **时序分析**：通过WCET分析验证所有截止时间
- **故障注入**：在故障条件下测试系统行为

### 4.3 EN 50128 (铁路控制软件)

- **静态分析**：100% MISRA-C:2012合规
- **动态分析**：安全关键代码100% MC/DC覆盖
- **黑盒测试**：使用无效输入测试所有接口

---

## 5. 安全架构模式

### 5.1 双核锁步（DCLS）

对于SIL 4 / ASIL D系统，使用双核锁步执行。

```c
/* ✅ 正确：DCLS模式 */
void safety_critical_function(uint32_t input, uint32_t *output) {
    uint32_t result_core0 = compute_on_core0(input);
    uint32_t result_core1 = compute_on_core1(input);

    if (result_core0 != result_core1) {
        /* 检测到不匹配 - 进入安全状态 */
        enter_safe_state();
        return;
    }

    *output = result_core0;
}
```

### 5.2 内存分区

使用MPU/MMU保护隔离安全关键组件。

```c
/* ✅ 正确：用于隔离的MPU配置 */
void mpu_init(void) {
    /* 区域0：安全关键内核（只读） */
    mpu_configure_region(0, KERNEL_BASE, KERNEL_SIZE,
        MPU_RO | MPU_EXECUTE_NEVER);

    /* 区域1：安全关键数据（不可执行） */
    mpu_configure_region(1, CRITICAL_DATA_BASE, CRITICAL_DATA_SIZE,
        MPU_RW | MPU_EXECUTE_NEVER);

    /* 区域2：非安全应用 */
    mpu_configure_region(2, APP_BASE, APP_SIZE,
        MPU_RW | MPU_EXECUTE);
}
```

### 5.3 看门狗定时器

始终为安全关键任务使用看门狗定时器。

```c
/* ✅ 正确：看门狗模式 */
void main_loop(void) {
    watchdog_init(1000);  /* 1秒超时 */

    for (;;) {
        watchdog_refresh();

        /* 执行安全关键任务 */
        if (!check_system_state()) {
            enter_safe_state();
        }

        if (!verify_outputs()) {
            enter_safe_state();
        }
    }
}
```

---

## 6. 安全状态管理

### 6.1 定义安全状态

每个系统必须具有**明确定义的安全状态**。

```c
/* ✅ 正确：安全状态定义 */
typedef enum {
    STATE_NORMAL = 0U,       /* 正常运行 */
    STATE_DEGRADED,          /* 降级模式 */
    STATE_SAFE_SHUTDOWN,     /* 安全关闭 */
    STATE_EMERGENCY_STOP     /* 紧急停止 */
} SystemState_t;

void enter_safe_state(SystemState_t state) {
    switch (state) {
        case STATE_SAFE_SHUTDOWN:
            /* 优雅关闭 */
            disable_all_outputs();
            save_state_to_nvm();
            power_down_non_essential();
            break;

        case STATE_EMERGENCY_STOP:
            /* 立即停止 */
            __asm__ volatile("msr daifset, #2");  /* 禁用IRQ */
            disable_all_outputs();
            enter_infinite_loop();
            break;

        default:
            break;
    }
}
```

### 6.2 故障检测

实施持续健康监控。

```c
/* ✅ 正确：健康检查模式 */
typedef struct {
    uint32_t task_id;
    uint64_t last_seen;
    uint32_t timeout_ms;
} TaskWatchdog_t;

bool task_health_check(TaskWatchdog_t *watchdog, uint32_t count) {
    uint64_t now = get_system_time_ms();

    for (uint32_t i = 0U; i < count; i++) {
        if ((now - watchdog[i].last_seen) > watchdog[i].timeout_ms) {
            /* 任务无响应 */
            return false;
        }
    }

    return true;
}
```

---

## 7. 文档要求

### 7.1 安全案例文档

每个安全关键组件必须具有：

1. **安全需求规范**
2. **安全架构描述**
3. **设计规范**
4. **验证报告**
5. **确认报告**
6. **安全分析**（FMEA、FTA）

### 7.2 代码注解

所有安全关键代码必须使用属性标记。

```c
/* ✅ 正确：安全注解 */
__attribute__((warn_unused_result))
ErrorCode_t safety_critical_function(uint32_t input);

/* 如果忽略返回值，编译器将警告 */
ErrorCode_t ret = safety_critical_function(42U);
```

### 7.3 语言规范要求

**所有代码注释、文档、规则文件必须使用中文**。

#### 7.3.1 代码注释语言

```c
/* ✅ 正确：使用中文注释说明代码功能 */
/**
 * @brief 创建新任务
 *
 * @param entry 任务入口函数指针（不能为NULL）
 * @param priority 任务优先级（0-255，255为最高）
 * @param stack_size 堆栈大小（字节，最小4096）
 *
 * @return 成功返回任务ID，失败返回0
 *
 * @note 必须在调度器启动前调用
 */
uint32_t task_create(void (*entry)(void), uint8_t priority, uint32_t stack_size);

/* ✅ 正确：行内注释使用中文 */
uint32_t task_id;  /* 任务唯一标识符 */
uint64_t system_ticks;  /* 系统时钟计数器 */

/* ❌ 错误：代码注释使用英文 */
/**
 * @brief Create new task
 * @param entry Task entry point
 * @param priority Task priority level
 */
uint32_t task_create(void (*entry)(void), uint8_t priority);
```

#### 7.3.2 规则文件语言

所有 `.claude/rules/` 下的规则文件必须使用中文描述：

```markdown
# ✅ 正确：规则文件使用中文
## LoongArch64编码规范

### 3.1 数据类型规范
- 必须使用固定宽度整数类型（int32_t, uint64_t等）
- 指针类型必须使用uintptr_t
```

#### 7.3.3 文档和问题回答

- 所有技术文档使用中文编写
- GitHub Issues、PR 讨论使用中文
- 代码审查评论使用中文
- 变量名和函数名可使用英文（技术术语）

#### 7.3.4 例外情况

仅在以下情况下允许使用英文：
1. **技术术语**：CPU、MMU、IRQ、API等
2. **标准名称**：MISRA-C:2012、IEC 61508、LoongArch等
3. **代码标识符**：函数名、变量名、类型名
4. **第三方库引用**：需要引用外部英文文档时

---

## 8. 禁止模式

以下模式在安全关键代码中**严格禁止**：

1. **递归** - 栈深度不可预测
2. **动态内存分配** - 堆碎片化
3. **变长数组** - 栈溢出风险
4. **指针运算** - 越界访问
5. **隐式类型转换** - 意外行为
6. **联合体类型双关** - 破坏严格别名
7. **goto语句** - 非结构化控制流
8. **setjmp/longjmp** - 破坏控制流假设
9. **信号处理函数** - 异步行为难以验证
10. **线程局部存储** - 复杂状态管理

---

## 总结

- **可预测性**：无未定义行为
- **显式性**：所有操作显式声明
- **可验证性**：所有代码可追溯和可测试
- **安全第一**：为SIL 4 / ASIL D设计
- **不走捷径**：MISRA-C:2012合规强制

记住：在安全关键系统中，**正确性**比**性能**更重要。
