/**
 * @file    cpu-info.h
 * @brief   LoongArch64 CPU信息结构定义
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件定义LoongArch64 CPU信息结构和相关常量
 *          - CPU信息结构体
 *          - Cache描述结构
 *          - ISA级别定义
 *          - CPU特性位定义
 *          - 处理器ID定义
 *          - FPU控制寄存器位定义
 *          - CPU特性检测内联函数
 *
 * @note MISRA-C:2012 合规
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef _CPU_INFO_H
#define _CPU_INFO_H

/************************头 文 件******************************/
#include <system/const.h>
#include <system/types.h>

/************************宏 定 义******************************/

/**
 * @defgroup CacheConfig Cache配置
 * @{
 */

/**
 * @brief L1 Cache移位值
 */
#define L1_CACHE_SHIFT        CONFIG_L1_CACHE_SHIFT

/**
 * @brief L1 Cache行大小（字节）
 */
#define L1_CACHE_BYTES        (1U << L1_CACHE_SHIFT)

/**
 * @brief SMP Cache行大小
 *
 * @details 默认使用L1 Cache行大小
 */
#ifndef SMP_CACHE_BYTES
#define SMP_CACHE_BYTES        L1_CACHE_BYTES
#endif

/** @} */

/**
 * @defgroup CacheFlags Cache标志位
 * @{
 */

/**
 * @brief Cache描述符标志位
 */
enum
{
    CACHE_PRESENT   = (1U << 0), /**< Cache存在 */
    CACHE_PRIVATE   = (1U << 1), /**< 核心私有Cache */
    CACHE_INCLUSIVE = (1U << 2), /**< 包含内层Cache */
};

/** @} */

/**
 * @defgroup CPUType CPU类型枚举
 * @{
 */

/**
 * @brief CPU类型枚举
 */
enum cpu_type_enum
{
    CPU_UNKNOWN,   /**< 未知CPU */
    CPU_LOONGSON32, /**< Loongson 32位CPU */
    CPU_LOONGSON64, /**< Loongson 64位CPU */
    CPU_LAST       /**< 枚举结束 */
};

/** @} */

/************************类型定义******************************/

/**
 * @brief Cache描述符结构
 *
 * @details 描述Cache的属性
 */
struct cache_desc
{
    unsigned char type;   /**< Cache类型 */
    unsigned char level;  /**< Cache级别 */
    unsigned short sets;  /**< 每组行数 */
    unsigned char ways;   /**< 路数 */
    unsigned char linesz; /**< 行大小（字节） */
    unsigned char flags;  /**< Cache属性标志 */
};

/**
 * @defgroup CacheConstants Cache常量
 * @{
 */
#define CACHE_LEVEL_MAX    3U /**< 最大Cache级别 */
#define CACHE_LEAVES_MAX   6U /**< 最大Cache叶子数 */
/** @} */

/**
 * @brief LoongArch CPU信息结构
 *
 * @details 描述LoongArch处理器的完整信息
 */
struct cpuinfo_loongarch
{
    u64 asid_cache;              /**< ASID缓存 */
    unsigned long asid_mask;     /**< ASID掩码 */

    /**
     * @name CPU能力和特性描述符
     */
    unsigned long long options;  /**< CPU特性选项位图 */
    unsigned int processor_id;   /**< 处理器ID */
    unsigned int fpu_vers;       /**< FPU版本 */
    unsigned int fpu_csr0;       /**< FPU控制状态寄存器0 */
    unsigned int fpu_mask;       /**< FPU掩码 */
    unsigned int cputype;        /**< CPU类型 */
    int isa_level;               /**< ISA级别 */
    int tlbsize;                 /**< TLB大小 */
    int tlbsizemtlb;             /**< MTLB大小 */
    int tlbsizestlbsets;         /**< STLB组数 */
    int tlbsizestlbways;         /**< STLB路数 */
    int cache_leaves_present;    /**< cache_leaves[]元素数量 */
    struct cache_desc cache_leaves[CACHE_LEAVES_MAX]; /**< Cache描述符数组 */
    int core;                    /**< Package中的物理核心号 */
    int package;                 /**< 物理Package号 */
    int global_id;               /**< 物理全局线程号 */
    int vabits;                  /**< 虚拟地址宽度（位） */
    int pabits;                  /**< 物理地址宽度（位） */
    int timerbits;               /**< 架构定时器宽度（位） */
    unsigned int ksave_mask;     /**< 可用的KSave掩码 */
    unsigned int watch_dreg_count;   /**< 数据断点数量 */
    unsigned int watch_ireg_count;   /**< 指令断点数量 */
    unsigned int watch_reg_use_cnt;  /**< 可用的观察点寄存器数量（用于ptrace） */
} __attribute__((__aligned__(SMP_CACHE_BYTES)));

/************************外部变量******************************/

/**
 * @brief CPU信息数组
 *
 * @details 每个CPU对应一个元素
 */
extern struct cpuinfo_loongarch cpu_data[];

/**
 * @brief 启动处理器CPU数据
 *
 * @details CPU0的别名
 */
#define boot_cpu_data    cpu_data[0U]

/**
 * @brief 当前处理器CPU数据
 *
 * @details 当前CPU的别名
 */
#define current_cpu_data    cpu_data[cpuid_get()]

/************************外部函数******************************/

/**
 * @brief CPU家族名称数组
 */
extern const char *__cpu_family[];

/**
 * @brief CPU完整名称数组
 */
extern const char *__cpu_full_name[];

/**
 * @brief 获取CPU家族名称
 */
#define cpu_family_string()    __cpu_family[cpuid_get()]

/**
 * @brief 获取CPU完整名称
 */
#define cpu_full_name_string()    __cpu_full_name[cpuid_get()]

/************************内联函数******************************/

/**
 * @defgroup ISALevel ISA级别编码
 * @{
 */

/**
 * @brief LoongArch ISA级别定义
 */
#define LOONGARCH_CPU_ISA_LA32R    0x00000001U /**< LA32r */
#define LOONGARCH_CPU_ISA_LA32S    0x00000002U /**< LA32s */
#define LOONGARCH_CPU_ISA_LA64     0x00000004U /**< LA64 */

/**
 * @brief 32位ISA级别
 */
#define LOONGARCH_CPU_ISA_32BIT    (LOONGARCH_CPU_ISA_LA32R | LOONGARCH_CPU_ISA_LA32S)

/**
 * @brief 64位ISA级别
 */
#define LOONGARCH_CPU_ISA_64BIT    LOONGARCH_CPU_ISA_LA64

/** @} */

/**
 * @defgroup ProcessorID 处理器ID定义
 * @{
 */

/**
 * @brief PRID寄存器公司值（位23:16）
 */
#define PRID_COMP_MASK        0xff0000U

/**
 * @brief Loongson公司ID
 */
#define PRID_COMP_LOONGSON    0x140000U

/**
 * @brief PRID寄存器系列ID（位15:12）
 */
#define PRID_SERIES_MASK      0xf000U

/**
 * @brief Loongson系列ID定义
 */
#define PRID_SERIES_LA132     0x8000U /**< Loongson 32位 */
#define PRID_SERIES_LA264     0xa000U /**< Loongson 64位，2发射 */
#define PRID_SERIES_LA364     0xb000U /**< Loongson 64位，3发射 */
#define PRID_SERIES_LA464     0xc000U /**< Loongson 64位，4发射 */
#define PRID_SERIES_LA664     0xd000U /**< Loongson 64位，6发射 */

/** @} */

/**
 * @defgroup CPUFeatures CPU特性位编码
 * @brief CPU特性位的编号定义
 * @{
 */
#define CPU_FEATURE_CPUCFG        0  /**< CPUCFG支持 */
#define CPU_FEATURE_LAM           1  /**< 原子指令支持 */
#define CPU_FEATURE_UAL           2  /**< 非对齐访问支持 */
#define CPU_FEATURE_FPU           3  /**< FPU支持 */
#define CPU_FEATURE_LSX           4  /**< LSX（128位SIMD）支持 */
#define CPU_FEATURE_LASX          5  /**< LASX（256位SIMD）支持 */
#define CPU_FEATURE_CRC32         6  /**< CRC32指令支持 */
#define CPU_FEATURE_COMPLEX       7  /**< 复杂指令支持 */
#define CPU_FEATURE_CRYPTO        8  /**< 加密指令支持 */
#define CPU_FEATURE_LVZ           9  /**< 虚拟化扩展支持 */
#define CPU_FEATURE_LBT_X86      10  /**< x86二进制翻译支持 */
#define CPU_FEATURE_LBT_ARM      11  /**< ARM二进制翻译支持 */
#define CPU_FEATURE_LBT_MIPS     12  /**< MIPS二进制翻译支持 */
#define CPU_FEATURE_TLB          13  /**< TLB支持 */
#define CPU_FEATURE_CSR          14  /**< CSR支持 */
#define CPU_FEATURE_IOCSR        15  /**< IOCSR支持 */
#define CPU_FEATURE_WATCH        16  /**< 观察点寄存器支持 */
#define CPU_FEATURE_VINT         17  /**< 向量化中断支持 */
#define CPU_FEATURE_CSRIPI       18  /**< CSR-IPI支持 */
#define CPU_FEATURE_EXTIOI       19  /**< EXT-IOI支持 */
#define CPU_FEATURE_PREFETCH     20  /**< 预取指令支持 */
#define CPU_FEATURE_PMP          21  /**< 性能计数器支持 */
#define CPU_FEATURE_SCALEFREQ    22  /**< 频率缩放支持 */
#define CPU_FEATURE_FLATMODE     23  /**< 平坦模式支持 */
#define CPU_FEATURE_EIODECODE    24  /**< EXTIOI中断引脚解码模式 */
#define CPU_FEATURE_GUESTID      25  /**< GuestID特性 */
#define CPU_FEATURE_HYPERVISOR   26  /**< 虚拟机监控程序（运行在VM中） */
#define CPU_FEATURE_PTW          27  /**< 硬件页表遍历器支持 */
#define CPU_FEATURE_LSPW         28  /**< LSPW（lddir/ldpte指令）支持 */
#define CPU_FEATURE_AVECINT      29  /**< AVEC中断支持 */

/**
 * @brief CPU特性位定义
 *
 * @details 将特性编号转换为位图
 */
#define LOONGARCH_CPU_CPUCFG      BIT_ULL(CPU_FEATURE_CPUCFG)
#define LOONGARCH_CPU_LAM         BIT_ULL(CPU_FEATURE_LAM)
#define LOONGARCH_CPU_UAL         BIT_ULL(CPU_FEATURE_UAL)
#define LOONGARCH_CPU_FPU         BIT_ULL(CPU_FEATURE_FPU)
#define LOONGARCH_CPU_LSX         BIT_ULL(CPU_FEATURE_LSX)
#define LOONGARCH_CPU_LASX        BIT_ULL(CPU_FEATURE_LASX)
#define LOONGARCH_CPU_CRC32       BIT_ULL(CPU_FEATURE_CRC32)
#define LOONGARCH_CPU_COMPLEX     BIT_ULL(CPU_FEATURE_COMPLEX)
#define LOONGARCH_CPU_CRYPTO      BIT_ULL(CPU_FEATURE_CRYPTO)
#define LOONGARCH_CPU_LVZ         BIT_ULL(CPU_FEATURE_LVZ)
#define LOONGARCH_CPU_LBT_X86     BIT_ULL(CPU_FEATURE_LBT_X86)
#define LOONGARCH_CPU_LBT_ARM     BIT_ULL(CPU_FEATURE_LBT_ARM)
#define LOONGARCH_CPU_LBT_MIPS    BIT_ULL(CPU_FEATURE_LBT_MIPS)
#define LOONGARCH_CPU_TLB         BIT_ULL(CPU_FEATURE_TLB)
#define LOONGARCH_CPU_IOCSR       BIT_ULL(CPU_FEATURE_IOCSR)
#define LOONGARCH_CPU_CSR         BIT_ULL(CPU_FEATURE_CSR)
#define LOONGARCH_CPU_WATCH       BIT_ULL(CPU_FEATURE_WATCH)
#define LOONGARCH_CPU_VINT        BIT_ULL(CPU_FEATURE_VINT)
#define LOONGARCH_CPU_CSRIPI      BIT_ULL(CPU_FEATURE_CSRIPI)
#define LOONGARCH_CPU_EXTIOI      BIT_ULL(CPU_FEATURE_EXTIOI)
#define LOONGARCH_CPU_PREFETCH    BIT_ULL(CPU_FEATURE_PREFETCH)
#define LOONGARCH_CPU_PMP         BIT_ULL(CPU_FEATURE_PMP)
#define LOONGARCH_CPU_SCALEFREQ   BIT_ULL(CPU_FEATURE_SCALEFREQ)
#define LOONGARCH_CPU_FLATMODE    BIT_ULL(CPU_FEATURE_FLATMODE)
#define LOONGARCH_CPU_EIODECODE   BIT_ULL(CPU_FEATURE_EIODECODE)
#define LOONGARCH_CPU_GUESTID     BIT_ULL(CPU_FEATURE_GUESTID)
#define LOONGARCH_CPU_HYPERVISOR  BIT_ULL(CPU_FEATURE_HYPERVISOR)
#define LOONGARCH_CPU_PTW         BIT_ULL(CPU_FEATURE_PTW)
#define LOONGARCH_CPU_LSPW        BIT_ULL(CPU_FEATURE_LSPW)
#define LOONGARCH_CPU_AVECINT     BIT_ULL(CPU_FEATURE_AVECINT)

/** @} */

/**
 * @defgroup HWCAP HWCAP标志
 * @brief 用于AT_HWCAP的硬件能力标志
 * @{
 */
#define HWCAP_LOONGARCH_CPUCFG     (1U << 0)
#define HWCAP_LOONGARCH_LAM        (1U << 1)
#define HWCAP_LOONGARCH_UAL        (1U << 2)
#define HWCAP_LOONGARCH_FPU        (1U << 3)
#define HWCAP_LOONGARCH_LSX        (1U << 4)
#define HWCAP_LOONGARCH_LASX       (1U << 5)
#define HWCAP_LOONGARCH_CRC32      (1U << 6)
#define HWCAP_LOONGARCH_COMPLEX    (1U << 7)
#define HWCAP_LOONGARCH_CRYPTO     (1U << 8)
#define HWCAP_LOONGARCH_LVZ        (1U << 9)
#define HWCAP_LOONGARCH_LBT_X86    (1U << 10)
#define HWCAP_LOONGARCH_LBT_ARM    (1U << 11)
#define HWCAP_LOONGARCH_LBT_MIPS   (1U << 12)
#define HWCAP_LOONGARCH_PTW        (1U << 13)
#define HWCAP_LOONGARCH_LSPW       (1U << 14)

/** @} */

/**
 * @defgroup Watchpoints 观察点寄存器数量
 * @{
 */

/**
 * @brief 固定观察点寄存器数量
 */
#define CSR_FWPC_NUM    0x3fU
#define CSR_MWPC_NUM    0x3fU

/** @} */

/**
 * @defgroup FPUCSR FPU控制状态寄存器位定义
 * @{
 */

/**
 * @brief FPU舍入模式位（位8-9）
 */
#define FPU_CSR_RM      0x300U
#define FPU_CSR_RN      0x000U /**< 最近舍入 */
#define FPU_CSR_RZ      0x100U /**< 向零舍入 */
#define FPU_CSR_RU      0x200U /**< 向+∞舍入 */
#define FPU_CSR_RD      0x300U /**< 向-∞舍入 */

/**
 * @brief FPU状态寄存器保留值
 */
#define FPU_CSR_RSVD    0xe0e0fce0U

/**
 * @name FPU异常标志
 * @brief X=异常原因指示，E=异常使能，S=粘性/标志位
 */
/*@{*/
#define FPU_CSR_ALL_X   0x1f000000U /**< 所有异常原因 */
#define FPU_CSR_INV_X   0x10000000U /**< 无效操作异常 */
#define FPU_CSR_DIV_X   0x08000000U /**< 除零异常 */
#define FPU_CSR_OVF_X   0x04000000U /**< 上溢异常 */
#define FPU_CSR_UDF_X   0x02000000U /**< 下溢异常 */
#define FPU_CSR_INE_X   0x01000000U /**< 不精确异常 */

#define FPU_CSR_ALL_S   0x001f0000U /**< 所有粘性标志 */
#define FPU_CSR_INV_S   0x00100000U /**< 无效操作标志 */
#define FPU_CSR_DIV_S   0x00080000U /**< 除零标志 */
#define FPU_CSR_OVF_S   0x00040000U /**< 上溢标志 */
#define FPU_CSR_UDF_S   0x00020000U /**< 下溢标志 */
#define FPU_CSR_INE_S   0x00010000U /**< 不精确标志 */

#define FPU_CSR_ALL_E   0x0000001FU /**< 所有异常使能 */
#define FPU_CSR_INV_E   0x00000010U /**< 无效操作使能 */
#define FPU_CSR_DIV_E   0x00000008U /**< 除零使能 */
#define FPU_CSR_OVF_E   0x00000004U /**< 上溢使能 */
#define FPU_CSR_UDF_E   0x00000002U /**< 下溢使能 */
#define FPU_CSR_INE_E   0x00000001U /**< 不精确使能 */
/*@}*/

/** @} */

/**
 * @brief 检测两个CPU是否为兄弟核心
 *
 * @details 检查两个CPU是否属于同一个物理核心
 *
 * @param cpua CPU A的ID
 * @param cpub CPU B的ID
 *
 * @return 如果是兄弟核心返回true，否则返回false
 */
static inline bool cpus_are_siblings(int cpua, int cpub)
{
    struct cpuinfo_loongarch *infoa = &cpu_data[cpua];
    struct cpuinfo_loongarch *infob = &cpu_data[cpub];

    if (infoa->package != infob->package)
    {
        return false;
    }

    if (infoa->core != infob->core)
    {
        return false;
    }

    return true;
}

/**
 * @brief 获取CPU的ASID掩码
 *
 * @param cpuinfo CPU信息结构指针
 *
 * @return 返回ASID掩码
 */
static inline unsigned long cpu_asid_mask(struct cpuinfo_loongarch *cpuinfo)
{
    return cpuinfo->asid_mask;
}

/**
 * @brief 设置CPU的ASID掩码
 *
 * @param cpuinfo   CPU信息结构指针
 * @param asid_mask ASID掩码值
 *
 * @return 无
 */
static inline void set_cpu_asid_mask(struct cpuinfo_loongarch *cpuinfo,
                                     unsigned long asid_mask)
{
    cpuinfo->asid_mask = asid_mask;
}

/**
 * @brief 获取断点寄存器数量
 *
 * @details 返回可用的断点（BRP）寄存器数量
 *
 * @return 返回断点寄存器数量
 */
static inline int get_num_brps(void)
{
    return (int)(csr_read64(LOONGARCH_CSR_FWPC) & CSR_FWPC_NUM);
}

/**
 * @brief 获取观察点寄存器数量
 *
 * @details 返回可用的观察点（WRP）寄存器数量
 *
 * @return 返回观察点寄存器数量
 */
static inline int get_num_wrps(void)
{
    return (int)(csr_read64(LOONGARCH_CSR_MWPC) & CSR_MWPC_NUM);
}

/************************C++兼容性******************************/
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif /* _CPU_INFO_H */
