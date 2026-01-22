/**
 * @file    cpu_info.c
 * @brief   LoongArch64 CPU信息探测
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.1
 *
 * @details 本文件实现LoongArch64 CPU信息探测功能
 *          - CPU特性探测
 *          - ISA级别检测
 *          - 缓存信息
 *          - TLB特性
 *          - FPU/SIMD支持
 *
 * @note MISRA-C:2012 合规
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

/*************************** 头文件包含 ****************************/
#include <cpu-features.h>
#include <cpu-info.h>
#include <cpu.h>
#include <driver/cpudev.h>
#include <limits.h>
#include <stdbool.h>

#undef KLOG_TAG
#define KLOG_TAG "CPU-INFO"
#include <klog.h>

/*************************** 宏定义 ****************************/
#define MAX_NAME_LEN 32
#define VENDOR_OFFSET 0
#define CPUNAME_OFFSET 9

/*************************** 类型定义 ****************************/

/*************************** 全局变量 ****************************/
extern void printk(const char *fmt, ...);

struct cpuinfo_loongarch cpu_data[CONFIG_MAX_CPUS];

/* Hardware capabilities */
unsigned int elf_hwcap;

const char *__cpu_family[CONFIG_MAX_CPUS];
const char *__cpu_full_name[CONFIG_MAX_CPUS];
const char *__elf_platform;

/* MAP BASE */
extern unsigned long vm_map_base;

/* Virtual Address size in bits */
extern unsigned long g_vaBits;

/* Physical Address size in bits */
extern unsigned long g_paBits;

/*************************** 外部声明 ****************************/

/*************************** 前向声明 ****************************/

/*************************** 模块变量 ****************************/
static char cpu_full_name[MAX_NAME_LEN] = "        -        ";

/*************************** 函数实现 ****************************/

/**
 * @brief 设置ELF平台
 *
 * @details 设置CPU的ELF平台标识
 *
 * @param cpu CPU编号
 * @param plat 平台名称字符串
 *
 * @return 无
 *
 * @note 仅在CPU0上设置全局平台标识
 */
static inline void set_elf_platform(int cpu, const char *plat)
{
    if (cpu == 0)
    {
        __elf_platform = plat;
    }

    return;
}

/**
 * @brief 探测地址宽度
 *
 * @details 从CPUCFG寄存器读取物理地址和虚拟地址的宽度
 *
 * @param 无
 *
 * @return 无
 *
 * @note 地址宽度用于计算虚拟内存映射基地址
 */
static void cpu_probe_addrbits(void)
{
    g_paBits = (read_cpucfg(LOONGARCH_CPUCFG1) & CPUCFG1_PABITS) >> 4;
    g_vaBits = (read_cpucfg(LOONGARCH_CPUCFG1) & CPUCFG1_VABITS) >> 12;
    vm_map_base = 0UL - (1UL << g_vaBits);

    return;
}

/**
 * @brief 设置ISA级别
 *
 * @details 设置CPU的指令集架构（ISA）级别
 *          支持LA64、LA32S、LA32R三种级别
 *
 * @param c CPU信息结构体指针
 * @param isa ISA级别
 *
 * @return 无
 *
 * @note LA64包含LA32S，LA32S包含LA32R（向下兼容）
 */
static void set_isa(struct cpuinfo_loongarch *c, unsigned int isa)
{
    switch (isa)
    {
        case LOONGARCH_CPU_ISA_LA64:
            c->isa_level |= LOONGARCH_CPU_ISA_LA64;
            __attribute__((__fallthrough__));
        case LOONGARCH_CPU_ISA_LA32S:
            c->isa_level |= LOONGARCH_CPU_ISA_LA32S;
            __attribute__((__fallthrough__));
        case LOONGARCH_CPU_ISA_LA32R:
            c->isa_level |= LOONGARCH_CPU_ISA_LA32R;
            break;
        default:
            break;
    }

    return;
}

/**
 * @brief 探测CPU公共特性
 *
 * @details 探测LoongArch64 CPU的基本特性和扩展功能
 *          包括分页、IOCSR、UAL、CRC32、FPU、LSX、LASX等
 *
 * @param c CPU信息结构体指针
 *
 * @return 无
 */
static void cpu_probe_common(struct cpuinfo_loongarch *c)
{
    unsigned int config;
    unsigned long asid_mask;

    c->options = LOONGARCH_CPU_CPUCFG | LOONGARCH_CPU_CSR | LOONGARCH_CPU_VINT;
    elf_hwcap = HWCAP_LOONGARCH_CPUCFG;

    config = read_cpucfg(LOONGARCH_CPUCFG1);

    /* 探测ISA级别 */
    switch (config & CPUCFG1_ISA)
    {
        case 0:
            set_isa(c, LOONGARCH_CPU_ISA_LA32R);
            break;
        case 1:
            set_isa(c, LOONGARCH_CPU_ISA_LA32S);
            break;
        case 2:
            set_isa(c, LOONGARCH_CPU_ISA_LA64);
            break;
        default:
            KLOG_W("Warning: unknown ISA level\n");
            break;
    }

    /* 探测分页支持 */
    if (config & CPUCFG1_PAGING)
    {
        c->options |= LOONGARCH_CPU_TLB;
    }

    /* 探测IOCSR支持 */
    if (config & CPUCFG1_IOCSR)
    {
        c->options |= LOONGARCH_CPU_IOCSR;
    }

    /* 探测非对齐访问支持 */
    if (config & CPUCFG1_UAL)
    {
        c->options |= LOONGARCH_CPU_UAL;
        elf_hwcap |= HWCAP_LOONGARCH_UAL;
    }

    /* 探测CRC32指令支持 */
    if (config & CPUCFG1_CRC32)
    {
        c->options |= LOONGARCH_CPU_CRC32;
        elf_hwcap |= HWCAP_LOONGARCH_CRC32;
    }

    config = read_cpucfg(LOONGARCH_CPUCFG2);

    /* 探测LAM支持 */
    if (config & CPUCFG2_LAM)
    {
        c->options |= LOONGARCH_CPU_LAM;
        elf_hwcap |= HWCAP_LOONGARCH_LAM;
    }

    /* 探测FPU支持 */
    if (config & CPUCFG2_FP)
    {
        c->options |= LOONGARCH_CPU_FPU;
        elf_hwcap |= HWCAP_LOONGARCH_FPU;
    }

#ifdef CONFIG_CPU_HAS_LSX
    /* 探测LSX支持 */
    if (config & CPUCFG2_LSX)
    {
        c->options |= LOONGARCH_CPU_LSX;
        elf_hwcap |= HWCAP_LOONGARCH_LSX;
    }
#endif

#ifdef CONFIG_CPU_HAS_LASX
    /* 探测LASX支持 */
    if (config & CPUCFG2_LASX)
    {
        c->options |= LOONGARCH_CPU_LASX;
        elf_hwcap |= HWCAP_LOONGARCH_LASX;
    }
#endif

    /* 探测复杂指令支持 */
    if (config & CPUCFG2_COMPLEX)
    {
        c->options |= LOONGARCH_CPU_COMPLEX;
        elf_hwcap |= HWCAP_LOONGARCH_COMPLEX;
    }

    /* 探测加密指令支持 */
    if (config & CPUCFG2_CRYPTO)
    {
        c->options |= LOONGARCH_CPU_CRYPTO;
        elf_hwcap |= HWCAP_LOONGARCH_CRYPTO;
    }

    /* 探测页表遍历支持 */
    if (config & CPUCFG2_PTW)
    {
        c->options |= LOONGARCH_CPU_PTW;
        elf_hwcap |= HWCAP_LOONGARCH_PTW;
    }

    /* 探测页表遍历同步支持 */
    if (config & CPUCFG2_LSPW)
    {
        c->options |= LOONGARCH_CPU_LSPW;
        elf_hwcap |= HWCAP_LOONGARCH_LSPW;
    }

    /* 探测虚拟化支持 */
    if (config & CPUCFG2_LVZP)
    {
        c->options |= LOONGARCH_CPU_LVZ;
        elf_hwcap |= HWCAP_LOONGARCH_LVZ;
    }

#ifdef CONFIG_CPU_HAS_LBT
    /* 探测二进制翻译支持 */
    if (config & CPUCFG2_X86BT)
    {
        c->options |= LOONGARCH_CPU_LBT_X86;
        elf_hwcap |= HWCAP_LOONGARCH_LBT_X86;
    }

    if (config & CPUCFG2_ARMBT)
    {
        c->options |= LOONGARCH_CPU_LBT_ARM;
        elf_hwcap |= HWCAP_LOONGARCH_LBT_ARM;
    }

    if (config & CPUCFG2_MIPSBT)
    {
        c->options |= LOONGARCH_CPU_LBT_MIPS;
        elf_hwcap |= HWCAP_LOONGARCH_LBT_MIPS;
    }
#endif

    config = read_cpucfg(LOONGARCH_CPUCFG6);

    /* 探测PMP支持 */
    if (config & CPUCFG6_PMP)
    {
        c->options |= LOONGARCH_CPU_PMP;
    }

    /* 探测ASID宽度 */
    config = csr_read32(LOONGARCH_CSR_ASID);
    config = (config & CSR_ASID_BIT) >> CSR_ASID_BIT_SHIFT;
    asid_mask = GENMASK(config - 1, 0);
    set_cpu_asid_mask(c, asid_mask);

    /* 探测定时器特性 */
    config = read_csr_prcfg1();
    c->timerbits = (config & CSR_CONF1_TMRBITS) >> CSR_CONF1_TMRBITS_SHIFT;
    c->ksave_mask = GENMASK((config & CSR_CONF1_KSNUM) - 1, 0);
    c->ksave_mask &= ~(EXC_KSAVE_MASK | PERCPU_KSAVE_MASK | KVM_KSAVE_MASK);

    /* 探测TLB类型 */
    config = read_csr_prcfg3();
    switch (config & CSR_CONF3_TLBTYPE)
    {
        case 0:
            c->tlbsizemtlb = 0;
            c->tlbsizestlbsets = 0;
            c->tlbsizestlbways = 0;
            c->tlbsize = 0;
            break;
        case 1:
            c->tlbsizemtlb = ((config & CSR_CONF3_MTLBSIZE) >> CSR_CONF3_MTLBSIZE_SHIFT) + 1;
            c->tlbsizestlbsets = 0;
            c->tlbsizestlbways = 0;
            c->tlbsize = c->tlbsizemtlb + c->tlbsizestlbsets * c->tlbsizestlbways;
            break;
        case 2:
            c->tlbsizemtlb = ((config & CSR_CONF3_MTLBSIZE) >> CSR_CONF3_MTLBSIZE_SHIFT) + 1;
            c->tlbsizestlbsets = 1 << ((config & CSR_CONF3_STLBIDX) >> CSR_CONF3_STLBIDX_SHIFT);
            c->tlbsizestlbways = ((config & CSR_CONF3_STLBWAYS) >> CSR_CONF3_STLBWAYS_SHIFT) + 1;
            c->tlbsize = c->tlbsizemtlb + c->tlbsizestlbsets * c->tlbsizestlbways;
            break;
        default:
            KLOG_W("Warning: unknown TLB type\n");
            break;
    }

    /* 探测观察点支持 */
    if (get_num_brps() + get_num_wrps())
    {
        c->options |= LOONGARCH_CPU_WATCH;
    }

    return;
}

/**
 * @brief 探测龙芯处理器信息
 *
 * @details 探测龙芯系列处理器的具体型号和特性
 *          包括LA132、LA264、LA364、LA464、LA664等
 *
 * @param c CPU信息结构体指针
 * @param cpu CPU编号
 *
 * @return 无
 *
 * @note 从IOCSR读取厂商和处理器名称信息
 */
static inline void cpu_probe_loongson(struct cpuinfo_loongarch *c, unsigned int cpu)
{
    uint32_t config;
    uint64_t *vendor = (void *)(&cpu_full_name[VENDOR_OFFSET]);
    uint64_t *cpuname = (void *)(&cpu_full_name[CPUNAME_OFFSET]);
    const char *core_name = "Unknown";

    /* 根据ISA级别设置处理器类型 */
    switch (BIT(fls(c->isa_level) - 1))
    {
        case LOONGARCH_CPU_ISA_LA32R:
        case LOONGARCH_CPU_ISA_LA32S:
            c->cputype = CPU_LOONGSON32;
            __cpu_family[cpu] = "Loongson-32bit";
            break;
        case LOONGARCH_CPU_ISA_LA64:
            c->cputype = CPU_LOONGSON64;
            __cpu_family[cpu] = "Loongson-64bit";
            break;
        default:
            printk("Warning: unknown ISA level\n");
            break;
    }

    /* 根据处理器ID识别具体型号 */
    switch (c->processor_id & PRID_SERIES_MASK)
    {
        case PRID_SERIES_LA132:
            core_name = "LA132";
            break;
        case PRID_SERIES_LA264:
            core_name = "LA264";
            break;
        case PRID_SERIES_LA364:
            core_name = "LA364";
            break;
        case PRID_SERIES_LA464:
            core_name = "LA464";
            break;
        case PRID_SERIES_LA664:
            core_name = "LA664";
            break;
        default:
            printk("Warning: unknown processor_id\n");
            break;
    }

    /* 从IOCSR读取厂商和处理器名称 */
    if (!cpu_has_iocsr)
    {
        return;
    }

    if (!__cpu_full_name[cpu])
    {
        __cpu_full_name[cpu] = cpu_full_name;
    }

    *vendor = iocsr_read64(LOONGARCH_IOCSR_VENDOR);
    *cpuname = iocsr_read64(LOONGARCH_IOCSR_CPUNAME);

    config = iocsr_read32(LOONGARCH_IOCSR_FEATURES);

    /* 探测扩展特性 */
    if (config & IOCSRF_CSRIPI)
    {
        c->options |= LOONGARCH_CPU_CSRIPI;
    }

    if (config & IOCSRF_EXTIOI)
    {
        c->options |= LOONGARCH_CPU_EXTIOI;
    }

    if (config & IOCSRF_FREQSCALE)
    {
        c->options |= LOONGARCH_CPU_SCALEFREQ;
    }

    if (config & IOCSRF_FLATMODE)
    {
        c->options |= LOONGARCH_CPU_FLATMODE;
    }

    if (config & IOCSRF_EIODECODE)
    {
        c->options |= LOONGARCH_CPU_EIODECODE;
    }

    if (config & IOCSRF_AVEC)
    {
        c->options |= LOONGARCH_CPU_AVECINT;
    }

    if (config & IOCSRF_VM)
    {
        c->options |= LOONGARCH_CPU_HYPERVISOR;
    }

    return;
}

/**
 * @brief 探测CPU信息
 *
 * @details 探测当前CPU的完整信息
 *          包括ISA级别、处理器类型、各种特性支持等
 *
 * @param 无
 *
 * @return 无
 *
 * @note 系统启动时为每个CPU调用此函数
 */
void cpu_probe(void)
{
    unsigned int cpu = cpuid_get();
    struct cpuinfo_loongarch *c = &current_cpu_data;

    /*
     * Set a default ELF platform, cpu probe may later
     * overwrite it with a more precise value
     */
    set_elf_platform(cpu, "loongarch");

    c->cputype = CPU_UNKNOWN;
    c->processor_id = read_cpucfg(LOONGARCH_CPUCFG0);
    c->fpu_vers = (read_cpucfg(LOONGARCH_CPUCFG2) & CPUCFG2_FPVERS) >> 3;
    c->fpu_csr0 = FPU_CSR_RN;
    c->fpu_mask = FPU_CSR_RSVD;

    cpu_probe_common(c);

    /* 根据厂商探测特定信息 */
    switch (c->processor_id & PRID_COMP_MASK)
    {
        case PRID_COMP_LOONGSON:
            cpu_probe_loongson(c, cpu);
            break;
        default:
            break;
    }

    cpu_probe_addrbits();

    return;
}
