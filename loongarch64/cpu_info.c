/**
 * @file    cpu_info.c
 * @brief   LoongArch64 CPU信息探测
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
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
/************************头 文 件******************************/
#include <cpu.h>
#include <limits.h>
#include <stdbool.h>
#include <cpu-info.h>
#include <cpu-features.h>
#include <driver/cpudev.h>
#undef KLOG_TAG
#define KLOG_TAG "CPU-INFO"
#include <klog.h>
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
static inline void set_elf_platform(int cpu, const char *plat)
{
	if (cpu == 0)
		__elf_platform = plat;
}
static void cpu_probe_addrbits()
{
	g_paBits = (read_cpucfg(LOONGARCH_CPUCFG1) & CPUCFG1_PABITS) >> 4;
	g_vaBits = (read_cpucfg(LOONGARCH_CPUCFG1) & CPUCFG1_VABITS) >> 12;
	vm_map_base = 0UL - (1UL << g_vaBits);
}
static void set_isa(struct cpuinfo_loongarch *c, unsigned int isa)
{
	switch (isa) {
	case LOONGARCH_CPU_ISA_LA64:
		c->isa_level |= LOONGARCH_CPU_ISA_LA64;
		__attribute__((__fallthrough__));
	case LOONGARCH_CPU_ISA_LA32S:
		c->isa_level |= LOONGARCH_CPU_ISA_LA32S;
		__attribute__((__fallthrough__));
	case LOONGARCH_CPU_ISA_LA32R:
		c->isa_level |= LOONGARCH_CPU_ISA_LA32R;
		break;
	}
}
static void cpu_probe_common(struct cpuinfo_loongarch *c)
{
	unsigned int config;
	unsigned long asid_mask;
	c->options = LOONGARCH_CPU_CPUCFG | LOONGARCH_CPU_CSR | LOONGARCH_CPU_VINT;
	elf_hwcap = HWCAP_LOONGARCH_CPUCFG;
	config = read_cpucfg(LOONGARCH_CPUCFG1);
	switch (config & CPUCFG1_ISA) {
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
	}
	if (config & CPUCFG1_PAGING)
		c->options |= LOONGARCH_CPU_TLB;
	if (config & CPUCFG1_IOCSR)
		c->options |= LOONGARCH_CPU_IOCSR;
	if (config & CPUCFG1_UAL) {
		c->options |= LOONGARCH_CPU_UAL;
		elf_hwcap |= HWCAP_LOONGARCH_UAL;
	}
	if (config & CPUCFG1_CRC32) {
		c->options |= LOONGARCH_CPU_CRC32;
		elf_hwcap |= HWCAP_LOONGARCH_CRC32;
	}
	config = read_cpucfg(LOONGARCH_CPUCFG2);
	if (config & CPUCFG2_LAM) {
		c->options |= LOONGARCH_CPU_LAM;
		elf_hwcap |= HWCAP_LOONGARCH_LAM;
	}
	if (config & CPUCFG2_FP) {
		c->options |= LOONGARCH_CPU_FPU;
		elf_hwcap |= HWCAP_LOONGARCH_FPU;
	}
#ifdef CONFIG_CPU_HAS_LSX
	if (config & CPUCFG2_LSX) {
		c->options |= LOONGARCH_CPU_LSX;
		elf_hwcap |= HWCAP_LOONGARCH_LSX;
	}
#endif
#ifdef CONFIG_CPU_HAS_LASX
	if (config & CPUCFG2_LASX) {
		c->options |= LOONGARCH_CPU_LASX;
		elf_hwcap |= HWCAP_LOONGARCH_LASX;
	}
#endif
	if (config & CPUCFG2_COMPLEX) {
		c->options |= LOONGARCH_CPU_COMPLEX;
		elf_hwcap |= HWCAP_LOONGARCH_COMPLEX;
	}
	if (config & CPUCFG2_CRYPTO) {
		c->options |= LOONGARCH_CPU_CRYPTO;
		elf_hwcap |= HWCAP_LOONGARCH_CRYPTO;
	}
	if (config & CPUCFG2_PTW) {
		c->options |= LOONGARCH_CPU_PTW;
		elf_hwcap |= HWCAP_LOONGARCH_PTW;
	}
	if (config & CPUCFG2_LSPW) {
		c->options |= LOONGARCH_CPU_LSPW;
		elf_hwcap |= HWCAP_LOONGARCH_LSPW;
	}
	if (config & CPUCFG2_LVZP) {
		c->options |= LOONGARCH_CPU_LVZ;
		elf_hwcap |= HWCAP_LOONGARCH_LVZ;
	}
#ifdef CONFIG_CPU_HAS_LBT
	if (config & CPUCFG2_X86BT) {
		c->options |= LOONGARCH_CPU_LBT_X86;
		elf_hwcap |= HWCAP_LOONGARCH_LBT_X86;
	}
	if (config & CPUCFG2_ARMBT) {
		c->options |= LOONGARCH_CPU_LBT_ARM;
		elf_hwcap |= HWCAP_LOONGARCH_LBT_ARM;
	}
	if (config & CPUCFG2_MIPSBT) {
		c->options |= LOONGARCH_CPU_LBT_MIPS;
		elf_hwcap |= HWCAP_LOONGARCH_LBT_MIPS;
	}
#endif
	config = read_cpucfg(LOONGARCH_CPUCFG6);
	if (config & CPUCFG6_PMP)
		c->options |= LOONGARCH_CPU_PMP;
	config = csr_read32(LOONGARCH_CSR_ASID);
	config = (config & CSR_ASID_BIT) >> CSR_ASID_BIT_SHIFT;
	asid_mask = GENMASK(config - 1, 0);
	set_cpu_asid_mask(c, asid_mask);
	config = read_csr_prcfg1();
	c->timerbits = (config & CSR_CONF1_TMRBITS) >> CSR_CONF1_TMRBITS_SHIFT;
	c->ksave_mask = GENMASK((config & CSR_CONF1_KSNUM) - 1, 0);
	c->ksave_mask &= ~(EXC_KSAVE_MASK | PERCPU_KSAVE_MASK | KVM_KSAVE_MASK);
	config = read_csr_prcfg3();
	switch (config & CSR_CONF3_TLBTYPE) {
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
	}
	if (get_num_brps() + get_num_wrps())
		c->options |= LOONGARCH_CPU_WATCH;
}
#define MAX_NAME_LEN	32
#define VENDOR_OFFSET	0
#define CPUNAME_OFFSET	9
static char cpu_full_name[MAX_NAME_LEN] = "        -        ";
static inline void cpu_probe_loongson(struct cpuinfo_loongarch *c, unsigned int cpu)
{
	uint32_t config;
	uint64_t *vendor = (void *)(&cpu_full_name[VENDOR_OFFSET]);
	uint64_t *cpuname = (void *)(&cpu_full_name[CPUNAME_OFFSET]);
	const char *core_name = "Unknown";
	switch (BIT(fls(c->isa_level) - 1)) {
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
	}
	switch (c->processor_id & PRID_SERIES_MASK) {
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
	}
	if (!cpu_has_iocsr)
		return;
	if (!__cpu_full_name[cpu])
		__cpu_full_name[cpu] = cpu_full_name;
	*vendor = iocsr_read64(LOONGARCH_IOCSR_VENDOR);
	*cpuname = iocsr_read64(LOONGARCH_IOCSR_CPUNAME);
	config = iocsr_read32(LOONGARCH_IOCSR_FEATURES);
	if (config & IOCSRF_CSRIPI)
		c->options |= LOONGARCH_CPU_CSRIPI;
	if (config & IOCSRF_EXTIOI)
		c->options |= LOONGARCH_CPU_EXTIOI;
	if (config & IOCSRF_FREQSCALE)
		c->options |= LOONGARCH_CPU_SCALEFREQ;
	if (config & IOCSRF_FLATMODE)
		c->options |= LOONGARCH_CPU_FLATMODE;
	if (config & IOCSRF_EIODECODE)
		c->options |= LOONGARCH_CPU_EIODECODE;
	if (config & IOCSRF_AVEC)
		c->options |= LOONGARCH_CPU_AVECINT;
	if (config & IOCSRF_VM)
		c->options |= LOONGARCH_CPU_HYPERVISOR;
}
void cpu_probe(void)
{
	unsigned int cpu = cpuid_get();
	struct cpuinfo_loongarch *c = &current_cpu_data;
	/*
	 * Set a default ELF platform, cpu probe may later
	 * overwrite it with a more precise value
	 */
	set_elf_platform(cpu, "loongarch");
	c->cputype	= CPU_UNKNOWN;
	c->processor_id = read_cpucfg(LOONGARCH_CPUCFG0);
	c->fpu_vers     = (read_cpucfg(LOONGARCH_CPUCFG2) & CPUCFG2_FPVERS) >> 3;
	c->fpu_csr0	= FPU_CSR_RN;
	c->fpu_mask	= FPU_CSR_RSVD;
	cpu_probe_common(c);
	switch (c->processor_id & PRID_COMP_MASK) {
	case PRID_COMP_LOONGSON:
		cpu_probe_loongson(c, cpu);
		break;
	}
	cpu_probe_addrbits();
}
