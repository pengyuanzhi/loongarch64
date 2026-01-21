/**
 * @file    generic_timer.c
 * @brief   LoongArch64通用定时器驱动
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件实现LoongArch64通用定时器驱动
 *          - 时钟源注册
 *          - 时钟事件设备
 *          - 定时器中断处理
 *          - 单次和周期模式
 *
 * @note MISRA-C:2012 合规
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */
/************************头 文 件******************************/
#include <barrier.h>
#include <clock/clockchip.h>
#include <clock/clocksource.h>
#include <cpu.h>
#include <cpuid.h>
#include <stdio.h>
#include <string.h>
#include <ttos_pic.h>
#include <ttos_time.h>
#include <kmalloc.h>
#include <time/ktime.h>
#undef DEBUG
#ifdef DEBUG
#define DPRINTF(msg...) KLOG_E (msg)
#else
#define DPRINTF(msg...)
#endif
#define KLOG_TAG "generic_timer.c"
#include <klog.h>
#define LOONGARCH_TIMER_PHYS_IRQ (11)
static u32 generic_timer_hz    = 0;
static u32 generic_timer_mult  = 0;
static u32 generic_timer_shift = 0;
static u32 timer_irq[4];
extern void sync_counter(void);
extern int constant_set_state_oneshot(void);
extern int constant_set_state_periodic(void);
static void generic_timer_get_freq(void)
{
    if (generic_timer_hz == 0)
    {
        generic_timer_hz = ttos_time_freq_get ();
    }
}
static u64 generic_counter_read(struct timer_clocksource *cs)
{
    return ttos_time_count_get ();
}
int arch_timer_clocksource_init(void)
{
    struct timer_clocksource *cs;
    generic_timer_get_freq ();
    if (generic_timer_hz == 0)
    {
        return -1;
    }
    cs = zalloc (sizeof (struct timer_clocksource));
    if (!cs)
    {
        return -1;
    }
    cs->name   = "gen-timer";
    cs->rating = 400;
    cs->read   = &generic_counter_read;
    cs->mask   = CLOCKSOURCE_MASK (64);
    cs->freq   = generic_timer_hz;
    clocks_calc_mult_shift (&cs->mult, &cs->shift, generic_timer_hz,
                            NSEC_PER_SEC, 10);
    generic_timer_mult  = cs->mult;
    generic_timer_shift = cs->shift;
    cs->priv            = NULL;
    return clocksource_register (cs);
}
static void generic_timer_stop(void)
{
    ttos_time_disable ();
}
static void generic_timer_set_mode(enum clockchip_mode mode,
                                   struct clockchip *cc)
{
    switch (mode)
    {
    case CLOCKCHIP_MODE_UNUSED:
    case CLOCKCHIP_MODE_SHUTDOWN:
        generic_timer_stop ();
        break;
    case CLOCKCHIP_MODE_PERIODIC:
        constant_set_state_periodic();
        break;
    case CLOCKCHIP_MODE_ONESHOT:
            constant_set_state_oneshot();
        break;       
    default:
        break;
    }
}
static int generic_timer_set_next_event(unsigned long long evt,
                                        struct clockchip *cc)
{
    int64_t ct = ttos_time_count_get() - evt;
    if (ct < (int64_t)cc->min_delta_ns)
        ct = cc->min_delta_ns;
    else if ((uint64_t)ct > cc->max_delta_ns)
        ct = cc->max_delta_ns;
    ttos_time_timeout_set(ct);    // 本次定时时间戳 - 当前时间戳 == 本次定时时间（nS）
    return 0;
}
static void generic_phys_timer_handler(uint32_t irq, void *dev)
{
    u32               ctl;
    struct clockchip *cc = dev;
    if (cc->bound_on != cpuid_get())
    {
        return;
    }
	/* Clear Timer Interrupt */
	write_csr_tintclear(CSR_TINTCLR_TI);
    cc->event_handler (cc);
    return;
}
static int generic_timer_startup (void)
{
    int               rc;
    u32               irq;
    struct clockchip *cc;
    /* Ensure ttos timer is stopped */
    generic_timer_stop ();
    /* Create generic ttos timer clockchip */
    cc = zalloc (sizeof (struct clockchip));
    if (!cc)
    {
        return -1;
    }
    cc->name     = "GENERIC_PHYSICAL_TIMER";
    cc->hirq     = LOONGARCH_TIMER_PHYS_IRQ;
    cc->rating   = 400;
    cc->features = CLOCKCHIP_FEAT_ONESHOT;
    cc->freq     = generic_timer_hz;
    clocks_calc_mult_shift (&cc->mult, &cc->shift, NSEC_PER_SEC,
                            generic_timer_hz, 3600);
    // cc->min_delta_ns   = clockchip_delta2ns (0xF, cc);
    cc->min_delta_ns   = clockchip_delta2ns (100, cc);
    cc->max_delta_ns   = clockchip_delta2ns (CLOCKSOURCE_MASK(48), cc);
    cc->set_mode       = &generic_timer_set_mode;
    cc->set_next_event = &generic_timer_set_next_event;
    cc->priv           = NULL;
    /* Register ttos timer clockchip */
    rc = clockchip_register (cc);
    if (rc)
    {
        free (cc);
        KLOG_EMERG("fail at %s:%d", __FILE__,__LINE__);
        return -1;
    }
    sync_counter();
    irq = ttos_core_pic_irq_alloc (LOONGARCH_TIMER_PHYS_IRQ);
    /* Register irq handler for timer */
    rc = ttos_pic_irq_install (irq, generic_phys_timer_handler, cc, IRQ_SHARED,
                               "PHYSICAL_TIMER");
    if (rc)
    {
        clockchip_unregister (cc);
        ttos_pic_irq_uninstall (irq, "PHYSICAL_TIMER");
        free (cc);
    }
	ttos_pic_irq_priority_set (irq, 0);
    ttos_pic_irq_unmask(irq);
    return 0;
}
int arch_timer_clockchip_init (void)
{
    /* Get and Check generic timer frequency */
    generic_timer_get_freq ();
    if (generic_timer_hz == 0)
    {
        return -1;
    }
    generic_timer_startup ();
    return 0;
}
