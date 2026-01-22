/**
 * @file    tlbexception.S
 * @brief   TLB异常处理汇编实现
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 本文件实现 LoongArch64 架构的 TLB 异常处理汇编函数
 *          - TLB 加载异常处理（handle_tlb_load）
 *          - TLB 存储异常处理（handle_tlb_store）
 *          - TLB 修改异常处理（handle_tlb_modify）
 *          - TLB 重填处理（handle_tlb_refill）
 *          - TLB 页表遍历异常处理（handle_tlb_load_ptw、handle_tlb_store_ptw、handle_tlb_modify_ptw）
 *          - TLB 保护异常处理（handle_tlb_protect0/1/2）
 *          - 大页（Huge Page）支持
 *          - SMP 多核页表同步支持
 *
 * @note MISRA-C:2012 合规
 * @warning 这些函数在异常上下文中执行，必须快速完成
 * @warning 必须正确处理页表权限和页面属性
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

/*************************** 修改历史 ****************************/
/*
 * 修改历史：
 * 2025-01-22    Intewell Team
 *               创建该文件
 */

/*************************** 头文件包含 ****************************/
#define ASM_USE
#include <cpu.h>
#include <asm-offsets.h>
#include <asm.h>
#include <pageTable.h>

/*************************** 宏定义 ****************************/
#define INVTLB_ADDR_GFALSE_AND_ASID    5
#define PTRS_PER_PGD_BITS    (PAGE_SIZE_SHIFT - 3)
#define PTRS_PER_PUD_BITS    (PAGE_SIZE_SHIFT - 3)
#define PTRS_PER_PMD_BITS    (PAGE_SIZE_SHIFT - 3)
#define PTRS_PER_PTE_BITS    (PAGE_SIZE_SHIFT - 3)

/*************************** 类型定义 ****************************/


/*************************** 外部声明 ****************************/


/*************************** 前向声明 ****************************/


/*************************** 模块变量 ****************************/


/*************************** 函数实现 ****************************/

/**
 * @brief TLB 页错误处理宏
 *
 * @details 统一的 TLB 页错误处理入口
 *          执行流程：
 *          1. 保存所有寄存器（SAVE_ALL）
 *          2. 读取出错地址（LOONGARCH_CSR_BADV）
 *          3. 调用 C 语言页错误处理函数（do_page_fault）
 *          4. 恢复所有寄存器并返回（RESTORE_ALL_AND_RET）
 *
 * @param write 是否为写操作（0=读，1=写）
 *
 * @note 此宏生成两个函数：tlb_do_page_fault_0 和 tlb_do_page_fault_1
 */
.macro tlb_do_page_fault, write
ENTRY(tlb_do_page_fault_\write)
    SAVE_ALL
    csrrd        a2, LOONGARCH_CSR_BADV      /* 读取出错地址 */
    move         a0, sp                       /* 参数1：异常栈指针 */
    REG_S        a2, sp, PT_BVADDR            /* 保存出错地址到栈帧 */
    li.w         a1, \write                   /* 参数2：是否写操作 */
    bl           do_page_fault                /* 调用页错误处理函数 */
    RESTORE_ALL_AND_RET
ENDPROC(tlb_do_page_fault_\write)
.endm

/* 生成读和写两个版本的页错误处理函数 */
tlb_do_page_fault 0
tlb_do_page_fault 1

/**
 * @brief TLB 保护异常处理0
 *
 * @details 处理 TLB 保护异常（类型0x3）
 *          执行流程：
 *          1. 保存所有寄存器
 *          2. 读取出错地址
 *          3. 设置错误类型为 0x3
 *          4. 调用页错误处理函数
 *          5. 恢复寄存器并返回
 *
 * @note 错误类型 0x3 表示页保护异常
 */
ENTRY(handle_tlb_protect0)
    SAVE_ALL
    move         a0, sp                       /* 参数1：异常栈指针 */
    li.w         a1, 0x3                      /* 参数2：错误类型 0x3 */
    csrrd        a2, LOONGARCH_CSR_BADV       /* 读取出错地址 */
    REG_S        a2, sp, PT_BVADDR            /* 保存出错地址 */
    la_abs       t0, do_page_fault
    jirl         ra, t0, 0                    /* 调用页错误处理函数 */
    RESTORE_ALL_AND_RET
ENDPROC(handle_tlb_protect0)

/**
 * @brief TLB 保护异常处理1
 *
 * @details 处理 TLB 保护异常（类型0x4）
 *          执行流程：
 *          1. 保存所有寄存器
 *          2. 读取出错地址
 *          3. 设置错误类型为 0x4
 *          4. 调用页错误处理函数
 *          5. 恢复寄存器并返回
 *
 * @note 错误类型 0x4 表示页保护异常
 */
ENTRY(handle_tlb_protect1)
    SAVE_ALL
    move         a0, sp                       /* 参数1：异常栈指针 */
    li.w         a1, 0x4                      /* 参数2：错误类型 0x4 */
    csrrd        a2, LOONGARCH_CSR_BADV       /* 读取出错地址 */
    REG_S        a2, sp, PT_BVADDR            /* 保存出错地址 */
    la_abs       t0, do_page_fault
    jirl         ra, t0, 0                    /* 调用页错误处理函数 */
    RESTORE_ALL_AND_RET
ENDPROC(handle_tlb_protect1)

/**
 * @brief TLB 保护异常处理2
 *
 * @details 处理 TLB 保护异常（类型0x5）
 *          执行流程：
 *          1. 保存所有寄存器
 *          2. 读取出错地址
 *          3. 设置错误类型为 0x5
 *          4. 调用页错误处理函数
 *          5. 恢复寄存器并返回
 *
 * @note 错误类型 0x5 表示页保护异常
 */
ENTRY(handle_tlb_protect2)
    SAVE_ALL
    move         a0, sp                       /* 参数1：异常栈指针 */
    li.w         a1, 0x5                      /* 参数2：错误类型 0x5 */
    csrrd        a2, LOONGARCH_CSR_BADV       /* 读取出错地址 */
    REG_S        a2, sp, PT_BVADDR            /* 保存出错地址 */
    la_abs       t0, do_page_fault
    jirl         ra, t0, 0                    /* 调用页错误处理函数 */
    RESTORE_ALL_AND_RET
ENDPROC(handle_tlb_protect2)

/**
 * @brief TLB 加载异常处理
 *
 * @details 处理 TLB 加载缺失异常（EXCCODE_TLBL）
 *          执行流程：
 *          1. 保存临时寄存器（t0, t1, ra）
 *          2. 检查是否为 vmalloc 地址（负地址）
 *          3. 如果是 vmalloc 地址，使用内核页目录
 *          4. 遍历页表（PGD -> PUD -> PMD -> PTE）
 *          5. 检查是否为大页（Huge Page）
 *          6. 如果是大页，进行大页处理
 *          7. 如果是普通页，读取 PTE 并检查权限
 *          8. 如果页存在，填充 TLB 并返回
 *          9. 如果页不存在，跳转到页错误处理
 *
 * @note 此函数在异常上下文中执行，必须快速完成
 * @note 支持 SMP 多核环境（使用 ll/sc 原子操作）
 * @note 支持大页（Huge Page）优化
 * @note 支持多级页表（3级或4级）
 */
ENTRY(handle_tlb_load)
    csrwr        t0, EXCEPTION_KS0           /* 保存 t0 */
    csrwr        t1, EXCEPTION_KS1           /* 保存 t1 */
    csrwr        ra, EXCEPTION_KS2           /* 保存 ra */

    /*
     * 检查是否为 vmalloc 地址（负地址表示内核虚拟地址）
     * vmalloc 处理不在热路径中
     */
    csrrd        t0, LOONGARCH_CSR_BADV      /* 读取出错地址 */
    bltz         t0, vmalloc_load            /* 如果是负地址，跳转到 vmalloc 处理 */
    csrrd        t1, LOONGARCH_CSR_PGDL      /* 读取页目录基址 */

vmalloc_done_load:
    /* 获取 PGD 偏移（字节） */
    bstrpick.d   ra, t0, PTRS_PER_PGD_BITS + PGDIR_SHIFT - 1, PGDIR_SHIFT
    alsl.d       t1, ra, t1, 3                /* t1 = &pgd[索引] */

#if CONFIG_PGTABLE_LEVELS > 3
    /* 4级页表：PUD */
    ld.d         t1, t1, 0
    bstrpick.d   ra, t0, PTRS_PER_PUD_BITS + PUD_SHIFT - 1, PUD_SHIFT
    alsl.d       t1, ra, t1, 3
#endif

#if CONFIG_PGTABLE_LEVELS > 2
    /* 3级或4级页表：PMD */
    ld.d         t1, t1, 0
    bstrpick.d   ra, t0, PTRS_PER_PMD_BITS + PMD_SHIFT - 1, PMD_SHIFT
    alsl.d       t1, ra, t1, 3
#endif

    /* 读取 PTE */
    ld.d         ra, t1, 0

    /*
     * 检查是否为大页（Huge Page）
     * 对于大页，PMD/PUD 不包含地址而是包含 TLB PTE
     * 检查 _PAGE_HUGE 位，如果设置则跳转到大页处理
     */
    rotri.d      ra, ra, _PAGE_HUGE_SHIFT + 1
    bltz         ra, tlb_huge_update_load     /* 跳转到大页处理 */
    rotri.d      ra, ra, 64 - (_PAGE_HUGE_SHIFT + 1)

    /* 普通页：获取 PTE 偏移 */
    bstrpick.d   t0, t0, PTRS_PER_PTE_BITS + PAGE_SIZE_SHIFT - 1, PAGE_SIZE_SHIFT
    alsl.d       t1, t0, ra, _PTE_T_LOG2      /* t1 = &pte[索引] */

#ifdef CONFIG_SMP
    /* SMP 多核环境：使用 Load-Linked/Store-Conditional 原子操作 */
smp_pgtable_change_load:
    ll.d         t0, t1, 0                    /* 加载链接 */
#else
    ld.d         t0, t1, 0                    /* 单核环境：直接加载 */
#endif

    /* 检查页是否存在 */
    andi         ra, t0, _PAGE_PRESENT
    beqz         ra, nopage_tlb_load          /* 页不存在，跳转到页错误处理 */

    /* 设置页有效位 */
    ori          t0, t0, _PAGE_VALID

#ifdef CONFIG_SMP
    sc.d         t0, t1, 0                    /* 条件存储 */
    beqz         t0, smp_pgtable_change_load  /* 失败则重试 */
#else
    st.d         t0, t1, 0                    /* 单核环境：直接存储 */
#endif

    /* 填充 TLB */
    tlbsrch                                  /* 搜索 TLB */
    bstrins.d    t1, zero, 3, 3               /* 清除位3（保留位） */
    ld.d         t0, t1, 0
    ld.d         t1, t1, 8
    csrwr        t0, LOONGARCH_CSR_TLBELO0    /* 设置 EntryLo0 */
    csrwr        t1, LOONGARCH_CSR_TLBELO1    /* 设置 EntryLo1 */
    tlbwr                                   /* 写入 TLB */

    /* 恢复寄存器并返回 */
    csrrd        t0, EXCEPTION_KS0
    csrrd        t1, EXCEPTION_KS1
    csrrd        ra, EXCEPTION_KS2
    ertn                                    /* 异常返回 */

#ifdef __loongarch64
    /* vmalloc 地址处理：使用内核页目录 */
vmalloc_load:
    la_abs       t1, kernel_pg_dir
    b            vmalloc_done_load
#endif

    /*
     * 大页（Huge Page）处理入口
     * 大页 PTE 描述的地址空间是配置的大页大小的两倍
     * 需要将大页 PTE 拆分为两个 TLB 条目（EntryLo0 和 EntryLo1）
     */
tlb_huge_update_load:
#ifdef CONFIG_SMP
    ll.d         ra, t1, 0                    /* 加载链接大页 PTE */
#else
    rotri.d      ra, ra, 64 - (_PAGE_HUGE_SHIFT + 1)
#endif

    /* 检查大页是否存在 */
    andi         t0, ra, _PAGE_PRESENT
    beqz         t0, nopage_tlb_load          /* 大页不存在 */

#ifdef CONFIG_SMP
    ori          t0, ra, _PAGE_VALID
    sc.d         t0, t1, 0                    /* 条件存储 */
    beqz         t0, tlb_huge_update_load     /* 失败则重试 */
    ori          t0, ra, _PAGE_VALID
#else
    ori          t0, ra, _PAGE_VALID
    st.d         t0, t1, 0
#endif

    /* 使无效旧的 TLB 条目 */
    csrrd        ra, LOONGARCH_CSR_ASID
    csrrd        t1, LOONGARCH_CSR_BADV
    andi         ra, ra, CSR_ASID_ASID
    invtlb       INVTLB_ADDR_GFALSE_AND_ASID, ra, t1

    /*
     * 大页处理：移动全局位
     * 大页 PTE 描述的区域大小为配置的大页大小
     * 这是我们要使用的大 TLB 条目大小的两倍
     * 在 entrylo0 和 entrylo1 中配置大 TLB 条目，
     * 覆盖连续的大 PTE 地址空间
     */
    xori         t0, t0, _PAGE_HUGE           /* 清除 HUGE 位 */
    lu12i.w      t1, _PAGE_HGLOBAL >> 12
    and          t1, t0, t1
    srli.d       t1, t1, (_PAGE_HGLOBAL_SHIFT - _PAGE_GLOBAL_SHIFT)
    or           t0, t0, t1
    move         ra, t0
    csrwr        ra, LOONGARCH_CSR_TLBELO0    /* 设置 EntryLo0 */

    /* 转换为 EntryLo1（添加大页大小偏移） */
    addi.d       t1, zero, 1
    slli.d       t1, t1, (HPAGE_SHIFT - 1)
    add.d        t0, t0, t1
    csrwr        t0, LOONGARCH_CSR_TLBELO1    /* 设置 EntryLo1 */

    /* 设置大页 TLB 条目大小 */
    addu16i.d    t0, zero, (CSR_TLBIDX_PS >> 16)
    addu16i.d    t1, zero, (PS_HUGE_SIZE << (CSR_TLBIDX_PS_SHIFT - 16))
    csrxchg      t1, t0, LOONGARCH_CSR_TLBIDX
    tlbfill                                /* 填充 TLB */

    /* 恢复默认页大小 */
    addu16i.d    t0, zero, (CSR_TLBIDX_PS >> 16)
    addu16i.d    t1, zero, (PS_DEFAULT_SIZE << (CSR_TLBIDX_PS_SHIFT - 16))
    csrxchg      t1, t0, LOONGARCH_CSR_TLBIDX

    /* 恢复寄存器并返回 */
    csrrd        t0, EXCEPTION_KS0
    csrrd        t1, EXCEPTION_KS1
    csrrd        ra, EXCEPTION_KS2
    ertn                                    /* 异常返回 */

nopage_tlb_load:
    dbar         0x700                        /* 数据屏障 */
    csrrd        ra, EXCEPTION_KS2
    la_abs       t0, tlb_do_page_fault_0
    jr           t0                           /* 跳转到页错误处理 */
ENDPROC(handle_tlb_load)

/**
 * @brief TLB 存储异常处理
 *
 * @details 处理 TLB 存储缺失异常（EXCCODE_TLBS）
 *          执行流程：
 *          1. 保存临时寄存器（t0, t1, ra）
 *          2. 检查是否为 vmalloc 地址
 *          3. 遍历页表（PGD -> PUD -> PMD -> PTE）
 *          4. 检查是否为大页
 *          5. 检查页是否可写
 *          6. 如果页可写，填充 TLB 并返回
 *          7. 如果页不可写，跳转到页错误处理
 *
 * @note 与 handle_tlb_load 类似，但需要检查写权限
 * @note 支持多核环境和大页
 */
ENTRY(handle_tlb_store)
    csrwr        t0, EXCEPTION_KS0
    csrwr        t1, EXCEPTION_KS1
    csrwr        ra, EXCEPTION_KS2

    /*
     * 检查是否为 vmalloc 地址
     * vmalloc 处理不在热路径中
     */
    csrrd        t0, LOONGARCH_CSR_BADV
    bltz         t0, vmalloc_store
    csrrd        t1, LOONGARCH_CSR_PGDL

vmalloc_done_store:
    /* 获取 PGD 偏移 */
    bstrpick.d   ra, t0, PTRS_PER_PGD_BITS + PGDIR_SHIFT - 1, PGDIR_SHIFT
    alsl.d       t1, ra, t1, 3

#if CONFIG_PGTABLE_LEVELS > 3
    ld.d         t1, t1, 0
    bstrpick.d   ra, t0, PTRS_PER_PUD_BITS + PUD_SHIFT - 1, PUD_SHIFT
    alsl.d       t1, ra, t1, 3
#endif

#if CONFIG_PGTABLE_LEVELS > 2
    ld.d         t1, t1, 0
    bstrpick.d   ra, t0, PTRS_PER_PMD_BITS + PMD_SHIFT - 1, PMD_SHIFT
    alsl.d       t1, ra, t1, 3
#endif

    ld.d         ra, t1, 0

    /*
     * 检查是否为大页
     */
    rotri.d      ra, ra, _PAGE_HUGE_SHIFT + 1
    bltz         ra, tlb_huge_update_store
    rotri.d      ra, ra, 64 - (_PAGE_HUGE_SHIFT + 1)

    bstrpick.d   t0, t0, PTRS_PER_PTE_BITS + PAGE_SIZE_SHIFT - 1, PAGE_SIZE_SHIFT
    alsl.d       t1, t0, ra, _PTE_T_LOG2

#ifdef CONFIG_SMP
smp_pgtable_change_store:
    ll.d         t0, t1, 0
#else
    ld.d         t0, t1, 0
#endif

    /* 检查页是否存在且可写 */
    andi         ra, t0, _PAGE_PRESENT | _PAGE_WRITE
    xori         ra, ra, _PAGE_PRESENT | _PAGE_WRITE
    bnez         ra, nopage_tlb_store         /* 页不存在或不可写 */

    /* 设置页有效、脏、修改位 */
    ori          t0, t0, (_PAGE_VALID | _PAGE_DIRTY | _PAGE_MODIFIED)

#ifdef CONFIG_SMP
    sc.d         t0, t1, 0
    beqz         t0, smp_pgtable_change_store
#else
    st.d         t0, t1, 0
#endif

    /* 填充 TLB */
    tlbsrch
    bstrins.d    t1, zero, 3, 3
    ld.d         t0, t1, 0
    ld.d         t1, t1, 8
    csrwr        t0, LOONGARCH_CSR_TLBELO0
    csrwr        t1, LOONGARCH_CSR_TLBELO1
    tlbwr

    /* 恢复寄存器并返回 */
    csrrd        t0, EXCEPTION_KS0
    csrrd        t1, EXCEPTION_KS1
    csrrd        ra, EXCEPTION_KS2
    ertn

#ifdef __loongarch64
vmalloc_store:
    la_abs       t1, kernel_pg_dir
    b            vmalloc_done_store
#endif

    /* 大页处理 */
tlb_huge_update_store:
#ifdef CONFIG_SMP
    ll.d         ra, t1, 0
#else
    rotri.d      ra, ra, 64 - (_PAGE_HUGE_SHIFT + 1)
#endif

    /* 检查大页是否存在且可写 */
    andi         t0, ra, _PAGE_PRESENT | _PAGE_WRITE
    xori         t0, t0, _PAGE_PRESENT | _PAGE_WRITE
    bnez         t0, nopage_tlb_store

#ifdef CONFIG_SMP
    ori          t0, ra, (_PAGE_VALID | _PAGE_DIRTY | _PAGE_MODIFIED)
    sc.d         t0, t1, 0
    beqz         t0, tlb_huge_update_store
    ori          t0, ra, (_PAGE_VALID | _PAGE_DIRTY | _PAGE_MODIFIED)
#else
    ori          t0, ra, (_PAGE_VALID | _PAGE_DIRTY | _PAGE_MODIFIED)
    st.d         t0, t1, 0
#endif

    /* 使无效旧的 TLB 条目 */
    csrrd        ra, LOONGARCH_CSR_ASID
    csrrd        t1, LOONGARCH_CSR_BADV
    andi         ra, ra, CSR_ASID_ASID
    invtlb       INVTLB_ADDR_GFALSE_AND_ASID, ra, t1

    /* 大页处理：移动全局位 */
    xori         t0, t0, _PAGE_HUGE
    lu12i.w      t1, _PAGE_HGLOBAL >> 12
    and          t1, t0, t1
    srli.d       t1, t1, (_PAGE_HGLOBAL_SHIFT - _PAGE_GLOBAL_SHIFT)
    or           t0, t0, t1
    move         ra, t0
    csrwr        ra, LOONGARCH_CSR_TLBELO0

    /* 转换为 EntryLo1 */
    addi.d       t1, zero, 1
    slli.d       t1, t1, (HPAGE_SHIFT - 1)
    add.d        t0, t0, t1
    csrwr        t0, LOONGARCH_CSR_TLBELO1

    /* 设置大页 TLB 条目大小 */
    addu16i.d    t0, zero, (CSR_TLBIDX_PS >> 16)
    addu16i.d    t1, zero, (PS_HUGE_SIZE << (CSR_TLBIDX_PS_SHIFT - 16))
    csrxchg      t1, t0, LOONGARCH_CSR_TLBIDX
    tlbfill

    /* 恢复默认页大小 */
    addu16i.d    t0, zero, (CSR_TLBIDX_PS >> 16)
    addu16i.d    t1, zero, (PS_DEFAULT_SIZE << (CSR_TLBIDX_PS_SHIFT - 16))
    csrxchg      t1, t0, LOONGARCH_CSR_TLBIDX

    /* 恢复寄存器并返回 */
    csrrd        t0, EXCEPTION_KS0
    csrrd        t1, EXCEPTION_KS1
    csrrd        ra, EXCEPTION_KS2
    ertn

nopage_tlb_store:
    dbar         0x700
    csrrd        ra, EXCEPTION_KS2
    la_abs       t0, tlb_do_page_fault_1
    jr           t0
ENDPROC(handle_tlb_store)

/**
 * @brief TLB 加载页表遍历异常处理
 *
 * @details 处理 TLB 加载时的页表遍历异常
 *          执行流程：
 *          1. 保存临时寄存器（t0, t1）
 *          2. 跳转到页错误处理
 *
 * @note 页表遍历异常表示页表项无效
 */
ENTRY(handle_tlb_load_ptw)
    csrwr        t0, LOONGARCH_CSR_KS0
    csrwr        t1, LOONGARCH_CSR_KS1
    la_abs       t0, tlb_do_page_fault_0
    jr           t0
ENDPROC(handle_tlb_load_ptw)

/**
 * @brief TLB 存储页表遍历异常处理
 *
 * @details 处理 TLB 存储时的页表遍历异常
 *          执行流程：
 *          1. 保存临时寄存器（t0, t1）
 *          2. 跳转到页错误处理
 *
 * @note 页表遍历异常表示页表项无效
 */
ENTRY(handle_tlb_store_ptw)
    csrwr        t0, LOONGARCH_CSR_KS0
    csrwr        t1, LOONGARCH_CSR_KS1
    la_abs       t0, tlb_do_page_fault_1
    jr           t0
ENDPROC(handle_tlb_store_ptw)

/**
 * @brief TLB 修改异常处理
 *
 * @details 处理 TLB 修改异常（EXCCODE_TLBM）
 *          执行流程：
 *          1. 保存临时寄存器（t0, t1, ra）
 *          2. 检查是否为 vmalloc 地址
 *          3. 遍历页表（PGD -> PUD -> PMD -> PTE）
 *          4. 检查是否为大页
 *          5. 检查页是否可写
 *          6. 如果页可写，填充 TLB 并返回
 *          7. 如果页不可写，跳转到页错误处理
 *
 * @note TLB 修改异常通常由写时复制（COW）触发
 * @note 与 handle_tlb_store 类似，但错误类型不同
 */
ENTRY(handle_tlb_modify)
    csrwr        t0, EXCEPTION_KS0
    csrwr        t1, EXCEPTION_KS1
    csrwr        ra, EXCEPTION_KS2

    /*
     * 检查是否为 vmalloc 地址
     * vmalloc 处理不在热路径中
     */
    csrrd        t0, LOONGARCH_CSR_BADV
    bltz         t0, vmalloc_modify
    csrrd        t1, LOONGARCH_CSR_PGDL

vmalloc_done_modify:
    /* 获取 PGD 偏移 */
    bstrpick.d   ra, t0, PTRS_PER_PGD_BITS + PGDIR_SHIFT - 1, PGDIR_SHIFT
    alsl.d       t1, ra, t1, 3

#if CONFIG_PGTABLE_LEVELS > 3
    ld.d         t1, t1, 0
    bstrpick.d   ra, t0, PTRS_PER_PUD_BITS + PUD_SHIFT - 1, PUD_SHIFT
    alsl.d       t1, ra, t1, 3
#endif

#if CONFIG_PGTABLE_LEVELS > 2
    ld.d         t1, t1, 0
    bstrpick.d   ra, t0, PTRS_PER_PMD_BITS + PMD_SHIFT - 1, PMD_SHIFT
    alsl.d       t1, ra, t1, 3
#endif

    ld.d         ra, t1, 0

    /*
     * 检查是否为大页
     */
    rotri.d      ra, ra, _PAGE_HUGE_SHIFT + 1
    bltz         ra, tlb_huge_update_modify
    rotri.d      ra, ra, 64 - (_PAGE_HUGE_SHIFT + 1)

    bstrpick.d   t0, t0, PTRS_PER_PTE_BITS + PAGE_SIZE_SHIFT - 1, PAGE_SIZE_SHIFT
    alsl.d       t1, t0, ra, _PTE_T_LOG2

#ifdef CONFIG_SMP
smp_pgtable_change_modify:
    ll.d         t0, t1, 0
#else
    ld.d         t0, t1, 0
#endif

    /* 检查页是否可写 */
    andi         ra, t0, _PAGE_WRITE
    beqz         ra, nopage_tlb_modify         /* 页不可写 */

    /* 设置页有效、脏、修改位 */
    ori          t0, t0, (_PAGE_VALID | _PAGE_DIRTY | _PAGE_MODIFIED)

#ifdef CONFIG_SMP
    sc.d         t0, t1, 0
    beqz         t0, smp_pgtable_change_modify
#else
    st.d         t0, t1, 0
#endif

    /* 填充 TLB */
    tlbsrch
    bstrins.d    t1, zero, 3, 3
    ld.d         t0, t1, 0
    ld.d         t1, t1, 8
    csrwr        t0, LOONGARCH_CSR_TLBELO0
    csrwr        t1, LOONGARCH_CSR_TLBELO1
    tlbwr

    /* 恢复寄存器并返回 */
    csrrd        t0, EXCEPTION_KS0
    csrrd        t1, EXCEPTION_KS1
    csrrd        ra, EXCEPTION_KS2
    ertn

#ifdef __loongarch64
vmalloc_modify:
    la_abs       t1, kernel_pg_dir
    b            vmalloc_done_modify
#endif

    /* 大页处理 */
tlb_huge_update_modify:
#ifdef CONFIG_SMP
    ll.d         ra, t1, 0
#else
    rotri.d      ra, ra, 64 - (_PAGE_HUGE_SHIFT + 1)
#endif

    /* 检查大页是否可写 */
    andi         t0, ra, _PAGE_WRITE
    beqz         t0, nopage_tlb_modify

#ifdef CONFIG_SMP
    ori          t0, ra, (_PAGE_VALID | _PAGE_DIRTY | _PAGE_MODIFIED)
    sc.d         t0, t1, 0
    beqz         t0, tlb_huge_update_modify
    ori          t0, ra, (_PAGE_VALID | _PAGE_DIRTY | _PAGE_MODIFIED)
#else
    ori          t0, ra, (_PAGE_VALID | _PAGE_DIRTY | _PAGE_MODIFIED)
    st.d         t0, t1, 0
#endif

    /* 使无效旧的 TLB 条目 */
    csrrd        ra, LOONGARCH_CSR_ASID
    csrrd        t1, LOONGARCH_CSR_BADV
    andi         ra, ra, CSR_ASID_ASID
    invtlb       INVTLB_ADDR_GFALSE_AND_ASID, ra, t1

    /* 大页处理：移动全局位 */
    xori         t0, t0, _PAGE_HUGE
    lu12i.w      t1, _PAGE_HGLOBAL >> 12
    and          t1, t0, t1
    srli.d       t1, t1, (_PAGE_HGLOBAL_SHIFT - _PAGE_GLOBAL_SHIFT)
    or           t0, t0, t1
    move         ra, t0
    csrwr        ra, LOONGARCH_CSR_TLBELO0

    /* 转换为 EntryLo1 */
    addi.d       t1, zero, 1
    slli.d       t1, t1, (HPAGE_SHIFT - 1)
    add.d        t0, t0, t1
    csrwr        t0, LOONGARCH_CSR_TLBELO1

    /* 设置大页 TLB 条目大小 */
    addu16i.d    t0, zero, (CSR_TLBIDX_PS >> 16)
    addu16i.d    t1, zero, (PS_HUGE_SIZE << (CSR_TLBIDX_PS_SHIFT - 16))
    csrxchg      t1, t0, LOONGARCH_CSR_TLBIDX
    tlbfill

    /* 恢复默认页大小 */
    addu16i.d    t0, zero, (CSR_TLBIDX_PS >> 16)
    addu16i.d    t1, zero, (PS_DEFAULT_SIZE << (CSR_TLBIDX_PS_SHIFT - 16))
    csrxchg      t1, t0, LOONGARCH_CSR_TLBIDX

    /* 恢复寄存器并返回 */
    csrrd        t0, EXCEPTION_KS0
    csrrd        t1, EXCEPTION_KS1
    csrrd        ra, EXCEPTION_KS2
    ertn

nopage_tlb_modify:
    dbar         0x700
    csrrd        ra, EXCEPTION_KS2
    la_abs       t0, tlb_do_page_fault_1
    jr           t0
ENDPROC(handle_tlb_modify)

/**
 * @brief TLB 修改页表遍历异常处理
 *
 * @details 处理 TLB 修改时的页表遍历异常
 *          执行流程：
 *          1. 保存临时寄存器（t0, t1）
 *          2. 跳转到页错误处理
 *
 * @note 页表遍历异常表示页表项无效
 */
ENTRY(handle_tlb_modify_ptw)
    csrwr        t0, LOONGARCH_CSR_KS0
    csrwr        t1, LOONGARCH_CSR_KS1
    la_abs       t0, tlb_do_page_fault_1
    jr           t0
ENDPROC(handle_tlb_modify_ptw)

/**
 * @brief TLB 重填处理
 *
 * @details 处理 TLB 重填异常（硬件自动页表遍历）
 *          执行流程：
 *          1. 保存 t0 到 TLBRSAVE
 *          2. 读取页目录基址（PGD）
 *          3. 使用硬件页表遍历指令（lddir、ldpte）
 *          4. 填充 TLB（tlbfill）
 *          5. 恢复 t0 并返回
 *
 * @note 此函数使用硬件页表遍历，比软件遍历更快
 * @note 仅在硬件支持时可用
 * @note lddir 指令自动遍历页表
 * @note ldpte 指令自动加载 PTE
 * @note tlbfill 指令自动填充 TLB
 */
ENTRY(handle_tlb_refill)
    csrwr        t0, LOONGARCH_CSR_TLBRSAVE   /* 保存 t0 到 TLBRSAVE */
    csrrd        t0, LOONGARCH_CSR_PGD        /* 读取页目录基址 */

    /* 硬件页表遍历 */
    lddir        t0, t0, 3                     /* 遍历 PGD -> PUD */

#if CONFIG_PGTABLE_LEVELS > 3
    lddir        t0, t0, 2                     /* 遍历 PUD -> PMD */
#endif

#if CONFIG_PGTABLE_LEVELS > 2
    lddir        t0, t0, 1                     /* 遍历 PMD -> PTE */
#endif

    /* 加载 PTE 并填充 TLB */
    ldpte        t0, 0                         /* 加载 PTE（偶数） */
    ldpte        t0, 1                         /* 加载 PTE（奇数） */
    tlbfill                                     /* 填充 TLB */

    /* 恢复并返回 */
    csrrd        t0, LOONGARCH_CSR_TLBRSAVE
    ertn                                    /* 异常返回 */
ENDPROC(handle_tlb_refill)
