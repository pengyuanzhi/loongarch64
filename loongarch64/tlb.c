#include <cpu.h>
#include <tlb.h>
#include <mmu.h>
#include <asm.h>
#include <exception.h>
#include <barrier.h>
#include <adrspace.h>
#include <ttosBase.h>
pgd_t kernel_pg_dir[PTRS_PER_PGD] __attribute__((__section__(".bss..kernel_pg_dir")));
pgd_t usr_pg_dir[PTRS_PER_PGD]__attribute__((__section__(".bss..page_aligned")))__attribute__((__aligned__(PAGE_SIZE)));
extern long exception_handlers[VECSIZE * 128 / sizeof(long)];
extern unsigned long eentry;
extern unsigned long tlbrentry;
void local_flush_tlb_all(void)
{
	invtlb_all(INVTLB_CURRENT_ALL, 0, 0);
}
void local_flush_tlb_user(void)
{
	invtlb_all(INVTLB_CURRENT_GFALSE, 0, 0);
}
void local_flush_tlb_kernel(void)
{
	invtlb_all(INVTLB_CURRENT_GTRUE, 0, 0);
}
/* 设置页表层级以及遍历方式,目前配置为4级页表PGD->PUD->PMD->PTE */
static void setup_ptwalker(void)
{
	unsigned long pwctl0 = 0, pwctl1 = 0;
	unsigned long pgd_i = 0, pgd_w = 0;
	unsigned long pud_i = 0, pud_w = 0;
	unsigned long pmd_i = 0, pmd_w = 0;
	unsigned long pte_i = 0, pte_w = 0;
	pgd_i = PGDIR_SHIFT;
	pgd_w = PAGE_SIZE_SHIFT - 3;
#if CONFIG_PGTABLE_LEVELS > 3
	pud_i = PUD_SHIFT;
	pud_w = PAGE_SIZE_SHIFT - 3;
#endif
#if CONFIG_PGTABLE_LEVELS > 2
	pmd_i = PMD_SHIFT;
	pmd_w = PAGE_SIZE_SHIFT - 3;
#endif
	pte_i = PAGE_SIZE_SHIFT;
	pte_w = PAGE_SIZE_SHIFT - 3;
	pwctl0 = pte_i | pte_w << 5 | pmd_i << 10 | pmd_w << 15 | pud_i << 20 | pud_w << 25;
	pwctl1 = pgd_i | pgd_w << 6;
	csr_write64(pwctl0, LOONGARCH_CSR_PWCTL0);
	csr_write64(pwctl1, LOONGARCH_CSR_PWCTL1);
	csr_write64(PHYSADDR((long)kernel_pg_dir), LOONGARCH_CSR_PGDH);
	csr_write64(PHYSADDR((long)usr_pg_dir), LOONGARCH_CSR_PGDL);
	local_flush_tlb_all();
	csr_write64((long)get_csr_cpuid(), LOONGARCH_CSR_TMID);
	//printk("pgd_i:[%d]-pud_i:[%d]-pmd_t:[%d] pte_i:[%d] \n\r",pgd_i,pud_i,pmd_i,pte_i);
	//printk("virtual---kernel_pg_dir:[0x%lx]-usr_pg_dir:[0x%lx] \n\r",kernel_pg_dir,usr_pg_dir);
	//printk("phy---kernel_pg_dir:[0x%lx]-usr_pg_dir:[0x%lx] \n\r",csr_read64(LOONGARCH_CSR_PGDH),csr_read64(LOONGARCH_CSR_PGDL));
}
void local_flush_icache_range(unsigned long start, unsigned long end);
void set_handler(unsigned long offset, void *addr, unsigned long size);
static void setup_tlb_handler(void)
{
	setup_ptwalker();
	local_flush_tlb_all();
	/* The tlb handlers are generated only once */
	if (get_csr_cpuid() == 0) 
    {
		memcpy((void *)tlbrentry, handle_tlb_refill, 0x80);
		local_flush_icache_range(tlbrentry, tlbrentry + 0x80);
		for (int i = EXCCODE_TLBL; i <= EXCCODE_TLBPE; i++)
			set_handler(i * VECSIZE, exception_table[i], VECSIZE);
	}
}
void tlb_init(void)
{
	/* 设置页表大小 */
	write_csr_pagesize(PS_4K);
	write_csr_stlbpgsize(PS_4K);
	write_csr_tlbrefill_pagesize(PS_4K);
	setup_tlb_handler();
}
/* debug api */
void show_tlb_info(unsigned long address)
{
	int idx;
	address &= (PAGE_MASK << 1);
	write_csr_entryhi(address);
	tlb_probe();
	idx = read_csr_tlbidx();
	printk("idx:[%d] \n\r",idx);
	tlb_read();
}
void show_exception_info()
{
	printk("TLB Refill");
	printk("****TLBRE EXCEPTION****\n\r");
	printk("TLBRBADV:[0x%lx] \n\r",csr_read64(LOONGARCH_CSR_TLBRBADV));
	printk("TLBRERA:[0x%lx] \n\r",csr_read64(LOONGARCH_CSR_TLBRERA));
	printk("EPC:[0x%lx] \n\r",csr_read64(LOONGARCH_CSR_TLBRERA)&0xfffffffffffffffc);
	printk("TLBRELO0:[0x%lx] \n\r",csr_read64(LOONGARCH_CSR_TLBRELO0));
	printk("TLBRELO1:[0x%lx] \n\r",csr_read64(LOONGARCH_CSR_TLBRELO1));
	printk("TLBREHI:[0x%lx] \n\r",csr_read64(LOONGARCH_CSR_TLBREHI));
	show_tlb_info(csr_read64(LOONGARCH_CSR_BADV));
	printk("****TLB INFO****\n\r");
	printk("TLBIDX:[0x%lx] \n\r",csr_read32(LOONGARCH_CSR_TLBIDX));
	printk("TLBEHI:[0x%lx] \n\r",csr_read64(LOONGARCH_CSR_TLBEHI));
	printk("TLBELO0:[0x%lx] \n\r",csr_read64(LOONGARCH_CSR_TLBELO0));
	printk("TLBELO1:[0x%lx] \n\r",csr_read64(LOONGARCH_CSR_TLBELO1));
}
void show_all_register(struct arch_context *ptRegs)
{
	printk("zero:[%p] \t",ptRegs->regs[0]);
	printk("ra:[%p] \t",ptRegs->regs[1]);
	printk("tp:[%p] \t",ptRegs->regs[2]);
	printk("sp:[%p] \t",ptRegs->regs[3]);
	printk("\n\r");
	printk("a0:[%p] \t",ptRegs->regs[4]);
	printk("a1:[%p] \t",ptRegs->regs[5]);
	printk("a2:[%p] \t",ptRegs->regs[6]);
	printk("a3:[%p] \t",ptRegs->regs[7]);
	printk("\n\r");
	printk("a4:[%p] \t",ptRegs->regs[8]);
	printk("a5:[%p] \t",ptRegs->regs[9]);
	printk("a6:[%p] \t",ptRegs->regs[10]);
	printk("a7:[%p] \t",ptRegs->regs[11]);
	printk("\n\r");
	printk("t0:[%p] \t",ptRegs->regs[12]);
	printk("t1:[%p] \t",ptRegs->regs[13]);
	printk("t2:[%p] \t",ptRegs->regs[14]);
	printk("t3:[%p] \t",ptRegs->regs[15]);
	printk("\n\r");
	printk("t4:[%p] \t",ptRegs->regs[16]);
	printk("t5:[%p] \t",ptRegs->regs[17]);
	printk("t6:[%p] \t",ptRegs->regs[18]);
	printk("t7:[%p] \t",ptRegs->regs[19]);
	printk("t8:[%p] \t",ptRegs->regs[20]);
	printk("\n\r");
	printk("fp:[%p] \t",ptRegs->regs[22]);
	printk("s0:[%p] \t",ptRegs->regs[23]);
	printk("s1:[%p] \t",ptRegs->regs[24]);
	printk("s2:[%p] \t",ptRegs->regs[25]);
	printk("\n\r");
	printk("s3:[%p] \t",ptRegs->regs[26]);
	printk("s4:[%p] \t",ptRegs->regs[27]);
	printk("s5:[%p] \t",ptRegs->regs[28]);
	printk("s6:[%p] \t",ptRegs->regs[29]);
	printk("\n\r");
	printk("s7:[%p] \t",ptRegs->regs[30]);
	printk("s8:[%p] \t",ptRegs->regs[31]);
	printk("\n\r");
	while(1);
}
void __attribute__((__section__(".kprobes.text"))) do_page_fault(struct arch_context *regs,
			unsigned long write, unsigned long address)
{
	printk("page fault:bad address is %p write:[%d]\n\r",address,write);
	printk("CSR_ERA:[%p] \n\r",regs->csr_era);
	printk("CSR_BADV:[%p] \n\r",regs->csr_badvaddr);
	printk("CSR_CRMD:[%p] \n\r",regs->csr_crmd);
	printk("CSR_PRMD:[%p] \n\r",regs->csr_prmd);
	printk("CSR_EUEN:[%p] \n\r",regs->csr_euen);
	printk("CSR_ECFG:[%p] \n\r",regs->csr_ecfg);
	printk("CSR_ESTAT:[%p] \n\r",regs->csr_estat);
	printk("CSR_PGDL:[%p] \n\r", csr_read64(LOONGARCH_CSR_PGDL));
	printk("CSR_ASID:[%p] \n\r", csr_read64(LOONGARCH_CSR_ASID));
    printk("Task name: %s (%p)\n", ttosGetRunningTaskName(), ttosGetRunningTask());
    printk("CPUID: %u\n", csr_read32(LOONGARCH_CSR_CPUID));
	show_exception_info();
	show_all_register(regs);
	while(1);
}
