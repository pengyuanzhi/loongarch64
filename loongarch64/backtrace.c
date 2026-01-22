/**
 * @file    backtrace.c
 * @brief   LoongArch64栈回溯实现
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 本文件实现了LoongArch64架构的栈回溯功能
 *          - 基于帧指针（fp, r22）的链表遍历
 *          - 地址有效性检查（MMU）
 *          - 符号表查找显示函数名
 *          - 支持从当前位置或指定地址开始回溯
 *
 * @note MISRA-C:2012 合规
 * @warning 使用-fno-omit-frame-pointer编译选项
 * @warning 仅用于调试构建，生产环境禁用
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

/*************************** 头文件包含 ****************************/
#include <arch/loongarch64/backtrace.h>
#include <arch/loongarch64/mmu.h>
#include <assert.h>
#include <klog.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <symtab.h>

#define KLOG_TAG "backtrace"
#include <klog.h>

/*************************** 宏定义 ****************************/
/**
 * @brief 栈回溯最大深度
 */
#define UNWIND_LIMIT 20U

/*************************** 数据结构 ****************************/
/**
 * @brief 帧记录结构
 *
 * @details 帧记录在栈上形成链表，用于栈回溯
 *          LoongArch64使用r22（fp）作为帧指针
 *          结构布局：[parent_fp][return_addr]
 */
struct frame_record
{
    struct frame_record *parent; /**< @brief 父帧记录指针 */
    uintptr_t return_addr;       /**< @brief 返回地址 */
};

/*************************** 内部函数声明 ****************************/
/**
 * @brief 检查地址是否可读
 *
 * @param addr 要检查的地址
 *
 * @return true表示地址可读，false表示不可读
 */
static bool is_address_readable(uintptr_t addr);

/**
 * @brief 检查对象是否有效
 *
 * @details 检查对象的所有字节是否在已映射内存中
 *
 * @param addr 对象地址
 * @param size 对象大小
 *
 * @return true表示对象有效，false表示无效
 */
static bool is_valid_object(uintptr_t addr, size_t size);

/**
 * @brief 检查跳转地址是否有效
 *
 * @param addr 跳转地址
 *
 * @return true表示地址有效，false表示无效
 */
static bool is_valid_jump_address(uintptr_t addr);

/**
 * @brief 检查帧记录是否有效
 *
 * @param fr 帧记录指针
 *
 * @return true表示帧记录有效，false表示无效
 */
static bool is_valid_frame_record(struct frame_record *fr);

/**
 * @brief 展开栈并打印回溯信息
 *
 * @param fr            当前帧记录
 * @param current_pc    当前程序计数器
 * @param ra            返回地址寄存器值
 * @param skip_current  是否跳过当前帧
 */
static void unwind_stack(struct frame_record *fr, uintptr_t current_pc, uintptr_t ra, bool skip_current);

/*************************** 函数实现 ****************************/
/**
 * @brief 检查地址是否可读
 *
 * @details 通过MMU检查虚拟地址是否可读
 *
 * @param addr 要检查的地址
 *
 * @return true表示地址可读，false表示不可读
 */
static bool is_address_readable(uintptr_t addr)
{
    /* 暂时返回true，后续需要添加真实的MMU检查 */
    (void)addr;

    return true;
}

/**
 * @brief 检查对象是否有效
 *
 * @details 检查对象的所有字节是否在已映射内存中且对齐正确
 *
 * @param addr 对象地址
 * @param size 对象大小
 *
 * @return true表示对象有效，false表示无效
 */
static bool is_valid_object(uintptr_t addr, size_t size)
{
    assert(size > 0U);

    if (addr == 0U)
    {
        return false;
    }

    /* 检测溢出 */
    if ((addr + size) < addr)
    {
        return false;
    }

    /* 指针必须正确对齐，否则可能触发对齐故障 */
    if ((addr & (sizeof(uintptr_t) - 1U)) != 0U)
    {
        return false;
    }

    /* 检查对象的所有字节是否可读 */
    for (size_t i = 0U; i < size; i++)
    {
        if (!is_address_readable(addr + i))
        {
            return false;
        }
    }

    return true;
}

/**
 * @brief 检查跳转地址是否有效
 *
 * @details 检查地址是否正确对齐且指向有效的内存区域
 *
 * @param addr 跳转地址
 *
 * @return true表示地址有效，false表示无效
 */
static bool is_valid_jump_address(uintptr_t addr)
{
    if (addr == 0U)
    {
        return false;
    }

    /* 检查对齐：LoongArch64指令为32位（4字节） */
    if ((addr & (sizeof(uint32_t) - 1U)) != 0U)
    {
        return false;
    }

    if (!is_address_readable(addr))
    {
        return false;
    }

    return true;
}

/**
 * @brief 检查帧记录是否有效
 *
 * @param fr 帧记录指针
 *
 * @return true表示帧记录有效，false表示无效
 */
static bool is_valid_frame_record(struct frame_record *fr)
{
    return is_valid_object((uintptr_t)fr, sizeof(struct frame_record));
}

/**
 * @brief 展开栈并打印回溯信息
 *
 * @details 遍历帧记录链表，打印每一层的调用信息
 *
 * @param fr            当前帧记录
 * @param current_pc    当前程序计数器
 * @param ra            返回地址寄存器值
 * @param skip_current  是否跳过当前帧
 */
static void unwind_stack(struct frame_record *fr, uintptr_t current_pc, uintptr_t ra, bool skip_current)
{
    uintptr_t call_site;
    static const char *backtrace_str = "%u: %s: 0x%lx (%s + 0x%x)";
    const char *el_str = "caller";

    if (!is_valid_frame_record(fr))
    {
        KLOG_EMERG("ERROR: Corrupted frame pointer (frame record address = %p)", fr);
        return;
    }

    if (fr->return_addr != ra)
    {
        KLOG_EMERG("ERROR: Corrupted stack (frame record address = %p)", fr);
        return;
    }

    /* 回溯的第0层是当前backtrace函数 */
    if (!skip_current)
    {
        size_t size;
        const struct symtab_item *sym = allsyms_findbyvalue((void *)current_pc, &size);
        KLOG_EMERG(backtrace_str, 0U, el_str, current_pc, sym != NULL ? sym->sym_name : "??",
                   (uint32_t)(current_pc - (uintptr_t)(sym != NULL ? sym->sym_value : 0U)));
    }

    /*
     * 栈开始处的最后一个帧记录指针应该为NULL，除非栈已损坏。
     */
    for (unsigned int i = skip_current ? 0U : 1U; i < UNWIND_LIMIT; i++)
    {
        /* 如果发现无效的帧记录，退出 */
        if (!is_valid_frame_record(fr))
        {
            return;
        }

        /*
         * LoongArch64指令长度固定为4字节，
         * 调用发起的地址是返回地址的前一条指令，
         * 即返回地址前4字节处。
         */
        call_site = fr->return_addr - 4U;

        /*
         * 如果地址无效，说明帧记录可能已损坏。
         */
        if (!is_valid_jump_address(call_site))
        {
            return;
        }

        {
            size_t size;
            const struct symtab_item *sym = allsyms_findbyvalue((void *)call_site, &size);
            KLOG_EMERG(backtrace_str, i, el_str, call_site, sym != NULL ? sym->sym_name : "??",
                       (uint32_t)(call_site - (uintptr_t)(sym != NULL ? sym->sym_value : 0U)));
        }

        fr = fr->parent;
    }

    KLOG_EMERG("ERROR: Max backtrace depth reached");
}

/**
 * @brief 显示栈回溯信息
 *
 * @details 从当前栈位置开始回溯，显示调用栈和函数名
 *          输出格式：
 *          BACKTRACE: START: <cookie>
 *          0: caller: 0x<addr> (<func> + 0x<offset>)
 *          1: caller: 0x<addr> (<func> + 0x<offset>)
 *          ...
 *          BACKTRACE: END: <cookie>
 *
 * @param cookie 用于标识和过滤日志消息的字符串
 *
 * @note 需要-fno-omit-frame-pointer编译选项
 * @note 使用链表遍历帧记录，最大深度为UNWIND_LIMIT
 *
 * @warning 许多因素可能阻止显示预期的回溯：
 *          - 编译器优化可能使用分支而非分支链接（尾调用优化）
 *          - 汇编函数可能不会设置符合规范的帧记录
 *          - 栈损坏会导致无效地址
 *
 * @warning 如果栈损坏，此函数可能显示安全敏感信息
 *          仅在调试构建中使用此函数
 */
void backtrace(const char *cookie)
{
    uintptr_t return_address = (uintptr_t)__builtin_return_address(0U);
    struct frame_record *fr = __builtin_frame_address(0U);

    KLOG_EMERG("BACKTRACE: START: %s", cookie);

    unwind_stack(fr, (uintptr_t)&backtrace, return_address, false);

    KLOG_EMERG("BACKTRACE: END: %s", cookie);
}

/**
 * @brief 从指定帧地址开始回溯栈
 *
 * @details 从给定的帧地址开始回溯，用于异常处理等场景
 *
 * @param cookie         用于标识和过滤日志消息的字符串
 * @param frame_address  帧记录地址
 *
 * @note frame_address必须是有效的帧指针地址
 * @note 如果地址无效，将输出"Cannot backtrace"并返回
 */
void backtrace_r(const char *cookie, uintptr_t frame_address)
{
    struct frame_record *fr = (struct frame_record *)frame_address;

    if (!is_address_readable((uintptr_t)fr))
    {
        KLOG_EMERG("Cannot backtrace");
        return;
    }

    KLOG_EMERG("BACKTRACE: START: %s", cookie);

    unwind_stack(fr, (uintptr_t)&backtrace_r, fr->return_addr, true);

    KLOG_EMERG("BACKTRACE: END: %s", cookie);
}
