# Intewell 实时操作系统 - LoongArch64 重构项目

## 项目概述

**Intewell** 是一个高性能的、原生的64位实时操作系统，专为 LoongArch64多核 SMP 架构设计。本项目遵循 **性能优先** 的设计理念，在确保安全性的前提下最大化系统吞吐量和响应速度。

### 关键特性

- **架构**：原生 LoongArch64，纯64位
- **多处理**：SMP（对称多处理），最多8核
- **调度**：256级优先级位图调度器，O(1)操作
- **内存管理**：启用 MMU，支持 4KB/2MB/1GB 页面
- **性能优先**：内核核心路径零开销
- **灵活规范**：MISRA-C:2012 性能优先版本

---

## 快速导航

### 核心标准

**[核心原则](rules/core-principles.md)**
- 安全关键编码原则
- 多核安全指南
- 功能安全要求
- 语言规范要求

**[MISRA-C:2012 规则](rules/misra-c2012.md)**
- 强制规则（1.1-21.1）
- 建议规则及理由
- 执行工具和脚本

### 编码标准

**[LoongArch64 编码规范](rules/loongarch-encoding.md)**
- 数据类型定义
- 对齐要求（16/64/4096 字节）
- 内联汇编模式
- 原子操作（C11 和 LoongArch64）
- 异常处理（PLV0-PLV3）
- 缓存操作
- MMU/TLB 管理

**[异常处理规范](rules/exception-handling.md)**
- 异常向量表
- 异常处理程序
- 中断处理
- 系统调用

**[代码风格](rules/code-style.md)**
- 命名约定（函数、变量、类型）
- 格式规则（Allman 风格，4空格缩进）
- 注释标准（Doxygen 风格）
- 文件组织模式

### 内存与并发

**[内存管理](rules/memory-management.md)**
- 动态分配安全
- 栈使用限制
- 堆保护（双重释放检测）
- 内存池管理

**[并发编程](rules/concurrency.md)**
- 锁使用（自旋锁、互斥锁）
- 无锁编程（SPSC 队列、栈）
- 死锁预防
- 内存屏障使用
- 多核SMP同步

### 工程实践

**[错误处理](rules/error-handling.md)**
- POSIX 错误码约定
- 错误处理模式
- 断言（编译时和运行时）
- 诊断要求

**[性能优化](rules/performance.md)**
- 内联函数指南
- 分支预测提示
- 缓存优化
- 数据结构布局

**[测试规范](rules/testing.md)**
- 单元测试结构（Unity 框架）
- 硬件模拟模式
- MC/DC 覆盖率要求
- 测试驱动开发工作流

**[检查清单](rules/checklist.md)**
- 提交前检查
- 代码审查检查

---

## 代码生成约束

### 性能优先原则

**核心理念**：内核核心路径（调度器、中断处理、上下文切换等）必须**零开销**，在性能关键路径中适当放宽非关键规则以确保最大执行效率。

### 分层规范要求

#### 1. 性能关键路径（内核核心）

```c
/* ✅ 性能关键路径：零开销，精简检查 */
- 调度器核心函数
- 中断处理函数
- 上下文切换
- 内存管理核心路径（MMU、页表）
- 同步原语（自旋锁、原子操作）

/* 特点：
 * - 无参数NULL检查（调用方保证有效性）
 * - 直接内存访问
 * - 最小化分支
 * - 内联关键函数
 */
```

#### 2. 公共API（用户接口）

```c
/* ✅ 公共API：严格检查，确保安全性 */
- 系统调用接口
- 设备驱动接口
- 文件系统接口
- 用户空间库函数

/* 特点：
 * - 完整参数验证
 * - 错误处理完善
 * - 符合MISRA-C:2012规则
 * - 提供详细文档
 */
```

### 核心要求

1. **性能优先**
   - 内核核心路径必须零开销
   - 热路径函数使用`static inline`
   - 编译器优化级别：`-O3 -flto`
   - 避免不必要的分支和检查

2. **LoongArch64 架构**
   - 纯64位（LoongArch指令集）
   - 16字节栈对齐
   - 共享数据使用原子操作
   - 正确的内存屏障（dbar/ibar）

3. **错误处理**
   - 使用 POSIX 错误码（EINVAL、ENOMEM 等）
   - 系统调用返回负错误码
   - 性能路径可简化检查（调用方保证有效性）

4. **代码风格**
   - Allman 括号风格
   - 4空格缩进（不使用 Tab）
   - 每行最多120字符
   - 公共 API 使用 Doxygen 注释

### 严格禁止模式（所有代码）

以下模式**所有代码**都必须遵守，包括性能关键路径：

- ❌ 禁止递归（MISRA 16.1）
- ❌ 禁止 goto 语句（MISRA 15.1）
- ❌ 禁止变长数组（MISRA 9.1）
- ❌ 禁止八进制常量（除 0 外）（MISRA 7.1）
- ❌ 禁止使用 `while(1)` - 应使用 `for(;;)`

### 可放宽模式（仅性能关键路径）

以下规则**仅在性能关键路径**中可适当放宽：

- ⚠️ 参数NULL检查（调用方保证有效性）
- ⚠️ 隐式类型转换（同宽度类型）
- ⚠️ 部分MISRA-C:2012建议规则

### 优先级顺序

当存在冲突时，遵循以下优先级顺序：

1. **性能**（核心路径零开销）← **新增最高优先级**
2. **安全**（无未定义行为）
3. **功能需求**（正确性）
4. **MISRA-C:2012**（合规性）
5. **代码风格**（可读性）

---

## 文件结构参考

```
Intewell/
├── .claude/
│   ├── CLAUDE.md              # 本文件
│   └── rules/                 # 编码规范
├── include/
│   └── arch/loongarch64/      # LoongArch64架构头文件
│       ├── cpu.h              # CPU定义
│       ├── asm.h              # 汇编宏
│       ├── exception.h        # 异常处理
│       ├── barrier.h          # 内存屏障
│       ├── mmu.h              # MMU管理
│       ├── tlb.h              # TLB维护
│       ├── ipi.h              # 核心间中断
│       └── ...
├── loongarch64/               # LoongArch64架构实现
│   ├── arch_init.c           # 架构初始化
│   ├── exception.c           # 异常处理
│   ├── atomic.c              # 原子操作
│   ├── cache.c               # 缓存管理
│   ├── mmu.c                 # MMU管理
│   ├── tlb.c                 # TLB维护
│   ├── ipi.c                 # IPI处理
│   └── ...
├── scripts/                   # 构建和测试脚本
├── tests/                     # 单元测试
└── docs/                      # 额外文档
```

---

## 快速参考

### 性能优化示例

```c
/* ✅ 性能关键路径：零开销实现 */
static inline void context_set_stack(T_TBSP_TaskContext *ctx, uint64_t sp)
{
    ctx->sp = (int64_t)sp;  /* 直接操作，无检查 */
}

/* ✅ 公共API：完整检查确保安全性 */
int32_t sys_write(int fd, const void *buf, size_t count)
{
    if (buf == NULL) {
        return -EINVAL;  /* 参数验证 */
    }
    if (count == 0) {
        return 0;
    }
    /* ... 完整错误处理 ... */
}

/* ❌ 错误：性能关键路径中过度检查 */
static inline void context_set_stack_slow(T_TBSP_TaskContext *ctx, uint64_t sp)
{
    if (ctx == NULL) {        /* ❌ 性能关键路径不应有此检查 */
        return -EINVAL;
    }
    if (sp & 0xF) {           /* ❌ 对齐检查应由调用方保证 */
        return -EINVAL;
    }
    ctx->sp = (int64_t)sp;
    return 0;
}
```

### 编译优化标志

```cmake
# 性能优化（Release配置）
-O3                 # 最高优化级别
-flto               # 链接时优化
-finline-limit=1000  # 内联扩展限制
-fomit-frame-pointer # 省略帧指针
-funroll-loops      # 循环展开
```

### 基础规范遵守

```c
/* ❌ 错误（所有代码都禁止） */
int x = 010;                    /* 八进制常量 */
while (1) { }                   /* 应使用 for (;;) */
goto error_handler;             /* 禁止goto */

/* ✅ 正确 */
int x = 10;                     /* 十进制 */
for (;;) { }                    /* 显式无限循环 */
if (error) {
    handle_error();
}
```

### LoongArch64 内存屏障

```c
barrier();          /* dbar 0 - 数据内存屏障 */
barrier_load();     /* dbar or_r_ - 加载屏障 */
barrier_store();    /* dbar o_w_w - 存储屏障 */
barrier_inst();     /* ibar 0 - 指令同步 */
full_barrier();     /* dbar + ibar - 完整屏障 */
```

### LoongArch64 内联汇编

```c
/* 读取CSR */
static inline uint64_t read_csr_pgdl(void)
{
    uint64_t value;
    __asm__ volatile("csrrd %0, 0x19" : "=r"(value));  /* LOONGARCH_CSR_PGDL */
    return value;
}

/* 写入CSR */
static inline void write_csr_pgdl(uint64_t value)
{
    __asm__ volatile("csrwr %0, 0x19" :: "r"(value));  /* LOONGARCH_CSR_PGDL */
    __asm__ volatile("ibar 0");
}
```

### POSIX 错误码

```c
/* 系统调用模式 */
long sys_read(int fd, void *buf, size_t count) {
    if (buf == NULL) {
        return -EINVAL;  /* 返回负错误码 */
    }
    /* ... */
    return bytes_read;   /* 成功时返回字节数 */
}
```

---

## 版本历史

- **v3.0** (2025-01-21): 性能优先版本
  - **性能优先**：内核核心路径零开销
  - **灵活规范**：MISRA-C:2012性能优先版本
  - **分层设计**：性能关键路径 vs 公共API

- **v2.0** (2025-01-21): LoongArch64重构项目
  - 精简为核心规则集
  - 添加LoongArch64特殊规范
  - 移除构建系统和配置系统规则

- **v1.0** (2025-01-09): 初始文档结构
  - 从单体 CLAUDE.md 提取
  - 模块化为 16 个专门的规则文件

---

## 联系与贡献

如有问题或贡献意向，请参考 `docs/` 中的项目文档。

**记住**：**性能第一**。内核核心路径必须零开销，在确保安全的前提下最大化执行效率。

---

## 语言规范

**所有代码注释、文档、规则文件必须使用中文。**

详细要求请参阅 [核心原则 - 语言规范要求](rules/core-principles.md#73-语言规范要求)。
