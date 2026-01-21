/**
 * @file    larchintrin.h
 * @brief   LoongArch64 GCC内联函数接口
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件定义LoongArch64 GCC编译器内联函数接口
 *          - 时间读取内联函数
 *          - FPU控制状态寄存器操作
 *          - 缓存操作
 *          - CPU配置读取
 *          - 页表遍历指令
 *          - CRC校验指令
 *          - CSR寄存器操作
 *          - IOCSR操作
 *          - 内存屏障指令
 *
 * @note MISRA-C:2012 合规
 * @note 这些是GCC内联函数，使用__builtin_loongarch_*内置函数
 * @note 基于GCC Runtime Library Exception v3.1发布
 *
 * @copyright Copyright (c) 2025 Intewell Team
 * @copyright Copyright (C) 2020-2022 Free Software Foundation, Inc.
 */

#ifndef _GCC_LOONGARCH_BASE_INTRIN_H
#define _GCC_LOONGARCH_BASE_INTRIN_H

/************************头 文 件******************************/

/************************宏 定 义******************************/

/************************类型定义******************************/

/**
 * @defgroup TimeStructures 时间读取结构
 * @{
 */

/**
 * @brief 双字时间读取结构
 *
 * @details 用于rdtime.d指令的返回值
 */
typedef struct drdtime
{
    unsigned long dvalue;  /**< 时间值 */
    unsigned long dtimeid; /**< 时间ID */
} __drdtime_t;

/**
 * @brief 字时间读取结构
 *
 * @details 用于rdtimeh.w/rdtimel.w指令的返回值
 */
typedef struct rdtime
{
    unsigned int value;  /**< 时间值 */
    unsigned int timeid; /**< 时间ID */
} __rdtime_t;

/** @} */

/************************外部变量******************************/
/************************外部函数******************************/

/************************内联函数******************************/

/**
 * @defgroup TimeIntrinsics 时间读取内联函数
 * @{
 */

/**
 * @brief 读取双字时间（64位）
 *
 * @details 使用rdtime.d指令读取稳定时钟
 *
 * @return 返回双字时间结构
 *
 * @note 仅在__loongarch64定义时可用
 */
#ifdef __loongarch64
extern __inline __drdtime_t __attribute__((__gnu_inline__, __always_inline__, __artificial__)) __rdtime_d(void)
{
    __drdtime_t __drdtime;
    __asm__ volatile("rdtime.d\t%[val],%[tid]\n\t" : [val] "=&r"(__drdtime.dvalue), [tid] "=&r"(__drdtime.dtimeid) :);
    return __drdtime;
}
#endif

/**
 * @brief 读取时间高位字
 *
 * @details 使用rdtimeh.w指令读取稳定时钟高位
 *
 * @return 返回字时间结构
 */
extern __inline __rdtime_t __attribute__((__gnu_inline__, __always_inline__, __artificial__)) __rdtimeh_w(void)
{
    __rdtime_t __rdtime;
    __asm__ volatile("rdtimeh.w\t%[val],%[tid]\n\t" : [val] "=&r"(__rdtime.value), [tid] "=&r"(__rdtime.timeid) :);
    return __rdtime;
}

/**
 * @brief 读取时间低位字
 *
 * @details 使用rdtimel.w指令读取稳定时钟低位
 *
 * @return 返回字时间结构
 */
extern __inline __rdtime_t __attribute__((__gnu_inline__, __always_inline__, __artificial__)) __rdtimel_w(void)
{
    __rdtime_t __rdtime;
    __asm__ volatile("rdtimel.w\t%[val],%[tid]\n\t" : [val] "=&r"(__rdtime.value), [tid] "=&r"(__rdtime.timeid) :);
    return __rdtime;
}

/** @} */

/**
 * @defgroup FPUIntrinsics FPU内联函数
 * @{
 */

/**
 * @brief 从FCSR移动到通用寄存器
 *
 * @details 将FPU控制状态寄存器的值读到通用寄存器
 *
 * @param _1 FCSR寄存器编号（0-3）
 *
 * @return 返回FCSR的值
 */
#define __movfcsr2gr(_1) __builtin_loongarch_movfcsr2gr((_1));

/**
 * @brief 从通用寄存器移动到FCSR
 *
 * @details 将值写入FPU控制状态寄存器
 *
 * @param _1 FCSR寄存器编号（0-3）
 * @param _2 要写入的值
 */
#define __movgr2fcsr(_1, _2) __builtin_loongarch_movgr2fcsr((_1), (unsigned int)_2);

/** @} */

/**
 * @defgroup CacheIntrinsics 缓存操作内联函数
 * @{
 */

/**
 * @brief 缓存操作（64位）
 *
 * @details 执行缓存维护操作
 *
 * @param _1 操作码
 * @param _2 地址
 * @param _3 偏移量（si12）
 */
#if defined __loongarch64
#define __cacop_d(_1, _2, _3) ((void)__builtin_loongarch_cacop_d((_1), (unsigned long int)(_2), (_3)))
#else
#error "Unsupported ABI."
#endif

/** @} */

/**
 * @defgroup CPUConfig CPU配置内联函数
 * @{
 */

/**
 * @brief 读取CPU配置
 *
 * @details 使用cpucfg指令读取处理器配置信息
 *
 * @param _1 配置选择器
 *
 * @return 返回配置值
 */
extern __inline unsigned int __attribute__((__gnu_inline__, __always_inline__, __artificial__))
__cpucfg(unsigned int _1)
{
    return (unsigned int)__builtin_loongarch_cpucfg((unsigned int)_1);
}

/** @} */

/**
 * @defgroup AssertionIntrinsics 断言内联函数
 * @{
 */

/**
 * @brief 小于等于断言（64位）
 *
 * @details 如果第一个操作数<=第二个操作数，则触发异常
 *
 * @param _1 第一个操作数
 * @param _2 第二个操作数
 */
#ifdef __loongarch64
extern __inline void
    __attribute__((__gnu_inline__, __always_inline__, __artificial__)) __asrtle_d(long int _1, long int _2)
{
    __builtin_loongarch_asrtle_d((long int)_1, (long int)_2);
}

/**
 * @brief 大于等于断言（64位）
 *
 * @details 如果第一个操作数>=第二个操作数，则触发异常
 *
 * @param _1 第一个操作数
 * @param _2 第二个操作数
 */
extern __inline void __attribute__((__gnu_inline__, __always_inline__, __artificial__)) __asrtgt_d(long int _1,
                                                                                                   long int _2)
{
    __builtin_loongarch_asrtgt_d((long int)_1, (long int)_2);
}
#endif

/** @} */

/**
 * @defgroup PageTableIntrinsics 页表遍历内联函数
 * @{
 */

/**
 * @brief 加载目录项（64位）
 *
 * @details 使用lddir.d指令遍历页表目录
 *
 * @param _1 地址
 * @param _2 层级
 *
 * @return 返回目录项地址
 */
#if defined __loongarch64
#define __lddir_d(_1, _2) ((long int)__builtin_loongarch_lddir_d((long int)(_1), (_2)))
#else
#error "Unsupported ABI."
#endif

/**
 * @brief 加载页表项（64位）
 *
 * @details 使用ldpte.d指令加载页表项
 *
 * @param _1 地址
 * @param _2 层级
 */
#if defined __loongarch64
#define __ldpte_d(_1, _2) ((void)__builtin_loongarch_ldpte_d((long int)(_1), (_2)))
#else
#error "Unsupported ABI."
#endif

/** @} */

/**
 * @defgroup CRCIntrinsics CRC校验内联函数
 * @{
 */

/**
 * @brief CRC字节数据（字）
 *
 * @param _1 字节数据
 * @param _2 CRC初始值
 *
 * @return 返回CRC结果
 */
extern __inline int __attribute__((__gnu_inline__, __always_inline__, __artificial__)) __crc_w_b_w(char _1, int _2)
{
    return (int)__builtin_loongarch_crc_w_b_w((char)_1, (int)_2);
}

/**
 * @brief CRC半字数据（字）
 *
 * @param _1 半字数据
 * @param _2 CRC初始值
 *
 * @return 返回CRC结果
 */
extern __inline int __attribute__((__gnu_inline__, __always_inline__, __artificial__)) __crc_w_h_w(short _1, int _2)
{
    return (int)__builtin_loongarch_crc_w_h_w((short)_1, (int)_2);
}

/**
 * @brief CRC字数据（字）
 *
 * @param _1 字数据
 * @param _2 CRC初始值
 *
 * @return 返回CRC结果
 */
extern __inline int __attribute__((__gnu_inline__, __always_inline__, __artificial__)) __crc_w_w_w(int _1, int _2)
{
    return (int)__builtin_loongarch_crc_w_w_w((int)_1, (int)_2);
}

/**
 * @brief CRC双字数据（字）
 *
 * @param _1 双字数据
 * @param _2 CRC初始值
 *
 * @return 返回CRC结果
 */
#ifdef __loongarch64
extern __inline int __attribute__((__gnu_inline__, __always_inline__, __artificial__)) __crc_w_d_w(long int _1, int _2)
{
    return (int)__builtin_loongarch_crc_w_d_w((long int)_1, (int)_2);
}
#endif

/**
 * @brief CRC字节数据（取反）
 *
 * @param _1 字节数据
 * @param _2 CRC初始值
 *
 * @return 返回CRC结果
 */
extern __inline int __attribute__((__gnu_inline__, __always_inline__, __artificial__)) __crcc_w_b_w(char _1, int _2)
{
    return (int)__builtin_loongarch_crcc_w_b_w((char)_1, (int)_2);
}

/**
 * @brief CRC半字数据（取反）
 *
 * @param _1 半字数据
 * @param _2 CRC初始值
 *
 * @return 返回CRC结果
 */
extern __inline int __attribute__((__gnu_inline__, __always_inline__, __artificial__)) __crcc_w_h_w(short _1, int _2)
{
    return (int)__builtin_loongarch_crcc_w_h_w((short)_1, (int)_2);
}

/**
 * @brief CRC字数据（取反）
 *
 * @param _1 字数据
 * @param _2 CRC初始值
 *
 * @return 返回CRC结果
 */
extern __inline int __attribute__((__gnu_inline__, __always_inline__, __artificial__)) __crcc_w_w_w(int _1, int _2)
{
    return (int)__builtin_loongarch_crcc_w_w_w((int)_1, (int)_2);
}

/**
 * @brief CRC双字数据（取反）
 *
 * @param _1 双字数据
 * @param _2 CRC初始值
 *
 * @return 返回CRC结果
 */
#ifdef __loongarch64
extern __inline int __attribute__((__gnu_inline__, __always_inline__, __artificial__)) __crcc_w_d_w(long int _1, int _2)
{
    return (int)__builtin_loongarch_crcc_w_d_w((long int)_1, (int)_2);
}
#endif

/** @} */

/**
 * @defgroup CSRIntrinsics CSR寄存器内联函数
 * @{
 */

/**
 * @brief 读取CSR寄存器（字）
 *
 * @param _1 CSR寄存器编号
 *
 * @return 返回CSR值
 */
#define __csrrd_w(_1) ((unsigned int)__builtin_loongarch_csrrd_w((_1)))

/**
 * @brief 写入CSR寄存器（字）
 *
 * @param _1 要写入的值
 * @param _2 CSR寄存器编号
 *
 * @return 返回写入的值
 */
#define __csrwr_w(_1, _2) ((unsigned int)__builtin_loongarch_csrwr_w((unsigned int)(_1), (_2)))

/**
 * @brief CSR交换操作（字）
 *
 * @param _1 新值
 * @param _2 旧值指针
 * @param _3 CSR寄存器编号
 *
 * @return 返回操作前的值
 */
#define __csrxchg_w(_1, _2, _3) \
    ((unsigned int)__builtin_loongarch_csrxchg_w((unsigned int)(_1), (unsigned int)(_2), (_3)))

/**
 * @brief 读取CSR寄存器（双字）
 *
 * @param _1 CSR寄存器编号
 *
 * @return 返回CSR值
 */
#ifdef __loongarch64
#define __csrrd_d(_1) ((unsigned long int)__builtin_loongarch_csrrd_d((_1)))

/**
 * @brief 写入CSR寄存器（双字）
 *
 * @param _1 要写入的值
 * @param _2 CSR寄存器编号
 *
 * @return 返回写入的值
 */
#define __csrwr_d(_1, _2) ((unsigned long int)__builtin_loongarch_csrwr_d((unsigned long int)(_1), (_2)))

/**
 * @brief CSR交换操作（双字）
 *
 * @param _1 新值
 * @param _2 旧值指针
 * @param _3 CSR寄存器编号
 *
 * @return 返回操作前的值
 */
#define __csrxchg_d(_1, _2, _3) \
    ((unsigned long int)__builtin_loongarch_csrxchg_d((unsigned long int)(_1), (unsigned long int)(_2), (_3)))
#endif

/** @} */

/**
 * @defgroup IOCSRIntrinsics IOCSR内联函数
 * @{
 */

/**
 * @brief 读取IOCSR（字节）
 *
 * @param _1 IOCSR端口
 *
 * @return 返回读取值
 */
extern __inline unsigned char __attribute__((__gnu_inline__, __always_inline__, __artificial__))
__iocsrrd_b(unsigned int _1)
{
    return (unsigned char)__builtin_loongarch_iocsrrd_b((unsigned int)_1);
}

/**
 * @brief 读取IOCSR（半字）
 *
 * @param _1 IOCSR端口
 *
 * @return 返回读取值
 */
extern __inline unsigned char __attribute__((__gnu_inline__, __always_inline__, __artificial__))
__iocsrrd_h(unsigned int _1)
{
    return (unsigned short)__builtin_loongarch_iocsrrd_h((unsigned int)_1);
}

/**
 * @brief 读取IOCSR（字）
 *
 * @param _1 IOCSR端口
 *
 * @return 返回读取值
 */
extern __inline unsigned int __attribute__((__gnu_inline__, __always_inline__, __artificial__))
__iocsrrd_w(unsigned int _1)
{
    return (unsigned int)__builtin_loongarch_iocsrrd_w((unsigned int)_1);
}

/**
 * @brief 读取IOCSR（双字）
 *
 * @param _1 IOCSR端口
 *
 * @return 返回读取值
 */
#ifdef __loongarch64
extern __inline unsigned long int
    __attribute__((__gnu_inline__, __always_inline__, __artificial__)) __iocsrrd_d(unsigned int _1)
{
    return (unsigned long int)__builtin_loongarch_iocsrrd_d((unsigned int)_1);
}
#endif

/**
 * @brief 写入IOCSR（字节）
 *
 * @param _1 要写入的值
 * @param _2 IOCSR端口
 */
extern __inline void __attribute__((__gnu_inline__, __always_inline__, __artificial__)) __iocsrwr_b(unsigned char _1,
                                                                                                    unsigned int _2)
{
    __builtin_loongarch_iocsrwr_b((unsigned char)_1, (unsigned int)_2);
}

/**
 * @brief 写入IOCSR（半字）
 *
 * @param _1 要写入的值
 * @param _2 IOCSR端口
 */
extern __inline void __attribute__((__gnu_inline__, __always_inline__, __artificial__)) __iocsrwr_h(unsigned short _1,
                                                                                                    unsigned int _2)
{
    __builtin_loongarch_iocsrwr_h((unsigned short)_1, (unsigned int)_2);
}

/**
 * @brief 写入IOCSR（字）
 *
 * @param _1 要写入的值
 * @param _2 IOCSR端口
 */
extern __inline void __attribute__((__gnu_inline__, __always_inline__, __artificial__)) __iocsrwr_w(unsigned int _1,
                                                                                                    unsigned int _2)
{
    __builtin_loongarch_iocsrwr_w((unsigned int)_1, (unsigned int)_2);
}

/**
 * @brief 写入IOCSR（双字）
 *
 * @param _1 要写入的值
 * @param _2 IOCSR端口
 */
#ifdef __loongarch64
extern __inline void __attribute__((__gnu_inline__, __always_inline__,
                                    __artificial__)) __iocsrwr_d(unsigned long int _1, unsigned int _2)
{
    __builtin_loongarch_iocsrwr_d((unsigned long int)_1, (unsigned int)_2);
}
#endif

/** @} */

/**
 * @defgroup BarrierIntrinsics 内存屏障内联函数
 * @{
 */

/**
 * @brief 数据屏障
 *
 * @details 插入数据内存屏障
 *
 * @param _1 屏蔽类型编码
 */
#define __dbar(_1) __builtin_loongarch_dbar((_1))

/**
 * @brief 指令屏障
 *
 * @details 插入指令内存屏障
 *
 * @param _1 屏蔽类型编码
 */
#define __ibar(_1) __builtin_loongarch_ibar((_1))

/** @} */

/**
 * @defgroup SystemIntrinsics 系统调用内联函数
 * @{
 */

/**
 * @brief 系统调用
 *
 * @details 触发系统调用异常
 *
 * @param _1 系统调用编号
 */
#define __syscall(_1) __builtin_loongarch_syscall((_1))

/**
 * @brief 断点异常
 *
 * @details 触发断点异常
 *
 * @param _1 断点编号
 */
#define __break(_1) __builtin_loongarch_break((_1))

/** @} */

/************************C++兼容性******************************/
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif /* _GCC_LOONGARCH_BASE_INTRIN_H */
