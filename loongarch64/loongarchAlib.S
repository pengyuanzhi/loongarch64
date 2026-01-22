/**
 * @file    loongarchAlib.S
 * @brief   LoongArch64 底层硬件访问库函数
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 本文件实现 LoongArch64 架构的底层硬件访问函数
 *          - HyperTransport 配置寄存器读写（32位）
 *          - 系统寄存器读写（64位、32位、8位）
 *          - 类型寄存器读写
 *          - 通过直接映射窗口访问物理内存
 *
 * @note MISRA-C:2012 合规
 * @warning 这些函数直接访问物理地址，必须谨慎使用
 * @warning 使用内存屏障确保访问顺序
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
#include <asm.h>
#include <cpu.h>
#include <system/const.h>

/*************************** 宏定义 ****************************/


/*************************** 类型定义 ****************************/


/*************************** 外部声明 ****************************/


/*************************** 前向声明 ****************************/


/*************************** 模块变量 ****************************/


/*************************** 函数实现 ****************************/

/**
 * @brief 读取 HyperTransport 配置寄存器（32位）
 *
 * @details 从指定的 HyperTransport 配置寄存器读取 32 位数据
 *          物理地址基址：0x80000EFDFB000000
 *
 * @param a0 offset 寄存器偏移量
 *
 * @return a0 读取到的 32 位数据
 *
 * @note 通过直接映射窗口访问物理地址
 * @note 使用数据内存屏障确保访问顺序
 */
ENTRY(LOONGSON3A_HT_CFG_LW32)
    li.d    t0, 0x80000EFDFB000000
    or      t0, t0, a0
    ld.w    a0, t0, 0
    dbar    0
    jirl    zero, ra, 0
ENDPROC(LOONGSON3A_HT_CFG_LW32)

/**
 * @brief 写入 HyperTransport 配置寄存器（32位）
 *
 * @details 向指定的 HyperTransport 配置寄存器写入 32 位数据
 *          物理地址基址：0x80000EFDFB000000
 *
 * @param a0 offset 寄存器偏移量
 * @param a1 value  要写入的 32 位数据
 *
 * @return 无
 *
 * @note 通过直接映射窗口访问物理地址
 * @note 使用数据内存屏障确保写入完成
 */
ENTRY(LOONGSON3A_HT_CFG_SW32)
    li.d    t0, 0x80000EFDFB000000
    or      t0, t0, a0
    st.w    a1, t0, 0
    dbar    0
    jirl    zero, ra, 0
ENDPROC(LOONGSON3A_HT_CFG_SW32)

/**
 * @brief 写入系统寄存器（8位）
 *
 * @details 向指定的系统寄存器写入 8 位数据
 *          物理地址基址：0x800000001fe00000
 *
 * @param a0 offset 寄存器偏移量
 * @param a1 value  要写入的 8 位数据
 *
 * @return 无
 *
 * @note 通过直接映射窗口访问物理地址
 * @note 使用数据内存屏障确保写入完成
 */
ENTRY(LOONGARCH_SB64_REGS)
    li.d    t0, 0x800000001fe00000
    or      t0, t0, a0
    st.b    a1, t0, 0
    dbar    0
    jirl    zero, ra, 0
ENDPROC(LOONGARCH_SB64_REGS)

/**
 * @brief 读取系统寄存器（8位）
 *
 * @details 从指定的系统寄存器读取 8 位数据
 *          物理地址基址：0x800000001fe00000
 *
 * @param a0 offset 寄存器偏移量
 *
 * @return a0 读取到的 8 位数据
 *
 * @note 通过直接映射窗口访问物理地址
 * @note 使用数据内存屏障确保访问顺序
 */
ENTRY(LOONGARCH_LB64_REGS)
    li.d    t0, 0x800000001fe00000
    or      t0, t0, a0
    ld.b    a0, t0, 0
    dbar    0
    jirl    zero, ra, 0
ENDPROC(LOONGARCH_LB64_REGS)

/**
 * @brief 写入系统寄存器（32位）
 *
 * @details 向指定的系统寄存器写入 32 位数据
 *          物理地址基址：0x800000001fe00000
 *
 * @param a0 offset 寄存器偏移量
 * @param a1 value  要写入的 32 位数据
 *
 * @return 无
 *
 * @note 通过直接映射窗口访问物理地址
 * @note 使用数据内存屏障确保写入完成
 */
ENTRY(LOONGARCH_SW32_REGS)
    li.d    t0, 0x800000001fe00000
    or      t0, t0, a0
    st.w    a1, t0, 0
    dbar    0
    jirl    zero, ra, 0
ENDPROC(LOONGARCH_SW32_REGS)

/**
 * @brief 读取系统寄存器（32位）
 *
 * @details 从指定的系统寄存器读取 32 位数据
 *          物理地址基址：0x800000001fe00000
 *
 * @param a0 offset 寄存器偏移量
 *
 * @return a0 读取到的 32 位数据
 *
 * @note 通过直接映射窗口访问物理地址
 * @note 使用数据内存屏障确保访问顺序
 */
ENTRY(LOONGARCH_LW32_REGS)
    li.d    t0, 0x800000001fe00000
    or      t0, t0, a0
    ld.w    a0, t0, 0
    dbar    0
    jirl    zero, ra, 0
ENDPROC(LOONGARCH_LW32_REGS)

/**
 * @brief 读取类型寄存器（64位）
 *
 * @details 从指定的类型寄存器读取数据（32位读取）
 *          物理地址基址：0x80000EFE00000000
 *
 * @param a0 offset 寄存器偏移量
 *
 * @return a0 读取到的数据
 *
 * @note 通过直接映射窗口访问物理地址
 * @note 使用数据内存屏障确保访问顺序
 */
ENTRY(LOONGARCH_LW64_TYPE)
    li.d    t0, 0x80000EFE00000000
    or      t0, t0, a0
    ld.w    a0, t0, 0
    dbar    0
    jirl    zero, ra, 0
ENDPROC(LOONGARCH_LW64_TYPE)

/**
 * @brief 写入类型寄存器（32位）
 *
 * @details 向指定的类型寄存器写入 32 位数据
 *          物理地址基址：0x80000EFE00000000
 *
 * @param a0 offset 寄存器偏移量
 * @param a1 value  要写入的 32 位数据
 *
 * @return 无
 *
 * @note 通过直接映射窗口访问物理地址
 * @note 使用数据内存屏障确保写入完成
 */
ENTRY(LOONGARCH_SW32_TYPE)
    li.d    t0, 0x80000EFE00000000
    or      t0, t0, a0
    st.w    a1, t0, 0
    dbar    0
    jirl    zero, ra, 0
ENDPROC(LOONGARCH_SW32_TYPE)

/**
 * @brief 写入物理内存（8位）
 *
 * @details 向指定的物理地址写入 8 位数据
 *          物理地址基址：0x8000000000000000
 *
 * @param a0 offset 物理地址偏移量
 * @param a1 value  要写入的 8 位数据
 *
 * @return 无
 *
 * @note 通过直接映射窗口访问物理地址
 * @note 使用数据内存屏障确保写入完成
 */
ENTRY(LOONGARCH_SB64)
    li.d    t0, 0x8000000000000000
    or      t0, t0, a0
    st.b    a1, t0, 0
    dbar    0
    jirl    zero, ra, 0
ENDPROC(LOONGARCH_SB64)

/**
 * @brief 写入物理内存（32位）
 *
 * @details 向指定的物理地址写入 32 位数据
 *          物理地址基址：0x8000000000000000
 *
 * @param a0 offset 物理地址偏移量
 * @param a1 value  要写入的 32 位数据
 *
 * @return 无
 *
 * @note 通过直接映射窗口访问物理地址
 * @note 使用数据内存屏障确保写入完成
 */
ENTRY(LOONGARCH_SW64)
    li.d    t0, 0x8000000000000000
    or      t0, t0, a0
    st.w    a1, t0, 0
    dbar    0
    jirl    zero, ra, 0
ENDPROC(LOONGARCH_SW64)

/**
 * @brief 读取物理内存（8位）
 *
 * @details 从指定的物理地址读取 8 位数据
 *          物理地址基址：0x8000000000000000
 *
 * @param a0 offset 物理地址偏移量
 *
 * @return a0 读取到的 8 位数据
 *
 * @note 通过直接映射窗口访问物理地址
 * @note 使用数据内存屏障确保访问顺序
 */
ENTRY(LOONGARCH_LB64)
    li.d    t0, 0x8000000000000000
    or      t0, t0, a0
    ld.b    a0, t0, 0
    dbar    0
    jirl    zero, ra, 0
ENDPROC(LOONGARCH_LB64)

/**
 * @brief 读取物理内存（32位）
 *
 * @details 从指定的物理地址读取 32 位数据
 *          物理地址基址：0x8000000000000000
 *
 * @param a0 offset 物理地址偏移量
 *
 * @return a0 读取到的 32 位数据
 *
 * @note 通过直接映射窗口访问物理地址
 * @note 使用数据内存屏障确保访问顺序
 */
ENTRY(LOONGARCH_LW64)
    li.d    t0, 0x8000000000000000
    or      t0, t0, a0
    ld.w    a0, t0, 0
    dbar    0
    jirl    zero, ra, 0
ENDPROC(LOONGARCH_LW64)
