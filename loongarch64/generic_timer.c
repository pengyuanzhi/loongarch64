/**
 * @file    generic_timer.c
 * @brief   LoongArch64通用定时器驱动
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.1
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

/*************************** 头文件包含 ****************************/
#include <barrier.h>
#include <clock/clockchip.h>
#include <clock/clocksource.h>
#include <cpu.h>
#include <cpuid.h>
#include <kmalloc.h>
#include <stdio.h>
#include <string.h>
#include <time/ktime.h>
#include <ttos_pic.h>
#include <ttos_time.h>

#undef DEBUG
#ifdef DEBUG
#define DPRINTF(msg...) KLOG_E(msg)
#else
#define DPRINTF(msg...)
#endif

#define KLOG_TAG "generic_timer.c"
#include <klog.h>

/*************************** 宏定义 ****************************/
#define LOONGARCH_TIMER_PHYS_IRQ (11)

/*************************** 类型定义 ****************************/

/*************************** 全局变量 ****************************/
static u32 generic_timer_hz = 0;
static u32 generic_timer_mult = 0;
static u32 generic_timer_shift = 0;
static u32 timer_irq[4];

/*************************** 外部声明 ****************************/
extern void sync_counter(void);
extern int constant_set_state_oneshot(void);
extern int constant_set_state_periodic(void);

/*************************** 前向声明 ****************************/

/*************************** 模块变量 ****************************/

/*************************** 函数实现 ****************************/

/**
 * @brief 获取通用定时器频率
 *
 * @details 读取并缓存通用定时器的频率值
 *
 * @param 无
 *
 * @return 无
 *
 * @note 仅在首次调用时读取频率，后续使用缓存值
 */
static void generic_timer_get_freq(void)
{
    if (generic_timer_hz == 0)
    {
        generic_timer_hz = ttos_time_freq_get();
    }

    return;
}

/**
 * @brief 读取通用计数器
 *
 * @details 读取当前通用计数器的值（用于时钟源）
 *
 * @param cs 时钟源结构体指针（未使用）
 *
 * @return 当前计数器值
 *
 * @note 此函数用于时钟子系统的时钟源
 */
static u64 generic_counter_read(struct timer_clocksource *cs)
{
    (void)cs;  /* 未使用的参数 */

    return ttos_time_count_get();
}

/**
 * @brief 初始化时钟源
 *
 * @details 注册通用定时器作为时钟源
 *          用于提供高精度时间戳
 *
 * @param 无
 *
 * @return 成功返回0，失败返回-1
 *
 * @note 时钟源用于跟踪系统时间
 */
int arch_timer_clocksource_init(void)
{
    struct timer_clocksource *cs;

    generic_timer_get_freq();

    if (generic_timer_hz == 0)
    {
        return -1;
    }

    cs = zalloc(sizeof(struct timer_clocksource));
    if (!cs)
    {
        return -1;
    }

    cs->name = "gen-timer";
    cs->rating = 400;
    cs->read = &generic_counter_read;
    cs->mask = CLOCKSOURCE_MASK(64);
    cs->freq = generic_timer_hz;
    clocks_calc_mult_shift(&cs->mult, &cs->shift, generic_timer_hz, NSEC_PER_SEC, 10);

    generic_timer_mult = cs->mult;
    generic_timer_shift = cs->shift;
    cs->priv = NULL;

    return clocksource_register(cs);
}

/**
 * @brief 停止通用定时器
 *
 * @details 禁用通用定时器
 *
 * @param 无
 *
 * @return 无
 *
 * @note 此函数用于关闭定时器
 */
static void generic_timer_stop(void)
{
    ttos_time_disable();

    return;
}

/**
 * @brief 设置定时器模式
 *
 * @details 设置通用定时器的工作模式
 *          支持关闭、周期性、单次三种模式
 *
 * @param mode 定时器模式
 *             - CLOCKCHIP_MODE_UNUSED: 未使用
 *             - CLOCKCHIP_MODE_SHUTDOWN: 关闭
 *             - CLOCKCHIP_MODE_PERIODIC: 周期性
 *             - CLOCKCHIP_MODE_ONESHOT: 单次
 * @param cc   时钟事件设备结构体指针（未使用）
 *
 * @return 无
 */
static void generic_timer_set_mode(enum clockchip_mode mode, struct clockchip *cc)
{
    (void)cc;  /* 未使用的参数 */

    switch (mode)
    {
        case CLOCKCHIP_MODE_UNUSED:
        case CLOCKCHIP_MODE_SHUTDOWN:
            generic_timer_stop();
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

    return;
}

/**
 * @brief 设置下一个定时器事件
 *
 * @details 设置下次定时器中断的时间戳
 *
 * @param evt 目标时间戳（纳秒）
 * @param cc  时钟事件设备结构体指针
 *
 * @return 成功返回0
 *
 * @note 计算相对时间并设置定时器
 */
static int generic_timer_set_next_event(unsigned long long evt, struct clockchip *cc)
{
    int64_t ct;

    ct = ttos_time_count_get() - evt;

    if (ct < (int64_t)cc->min_delta_ns)
    {
        ct = cc->min_delta_ns;
    }
    else if ((uint64_t)ct > cc->max_delta_ns)
    {
        ct = cc->max_delta_ns;
    }

    ttos_time_timeout_set(ct);

    return 0;
}

/**
 * @brief 定时器中断处理函数
 *
 * @details 处理通用定时器的中断事件
 *          清除中断标志并调用事件处理回调
 *
 * @param irq 中断号
 * @param dev 设备私有数据（时钟事件设备指针）
 *
 * @return 无
 *
 * @note 仅在绑定CPU上处理中断
 */
static void generic_phys_timer_handler(uint32_t irq, void *dev)
{
    u32 ctl;
    struct clockchip *cc = dev;

    (void)irq;  /* 未使用的参数 */

    if (cc->bound_on != cpuid_get())
    {
        return;
    }

    /* Clear Timer Interrupt */
    write_csr_tintclear(CSR_TINTCLR_TI);
    cc->event_handler(cc);

    return;
}

/**
 * @brief 启动通用定时器
 *
 * @details 初始化并注册通用定时器作为时钟事件设备
 *          配置中断处理函数并使能中断
 *
 * @param 无
 *
 * @return 成功返回0，失败返回-1
 *
 * @note 此函数完成定时器的完整初始化流程
 */
static int generic_timer_startup(void)
{
    int rc;
    u32 irq;
    struct clockchip *cc;

    /* Ensure ttos timer is stopped */
    generic_timer_stop();

    /* Create generic ttos timer clockchip */
    cc = zalloc(sizeof(struct clockchip));
    if (!cc)
    {
        return -1;
    }

    cc->name = "GENERIC_PHYSICAL_TIMER";
    cc->hirq = LOONGARCH_TIMER_PHYS_IRQ;
    cc->rating = 400;
    cc->features = CLOCKCHIP_FEAT_ONESHOT;
    cc->freq = generic_timer_hz;
    clocks_calc_mult_shift(&cc->mult, &cc->shift, NSEC_PER_SEC, generic_timer_hz, 3600);
    cc->min_delta_ns = clockchip_delta2ns(100, cc);
    cc->max_delta_ns = clockchip_delta2ns(CLOCKSOURCE_MASK(48), cc);
    cc->set_mode = &generic_timer_set_mode;
    cc->set_next_event = &generic_timer_set_next_event;
    cc->priv = NULL;

    /* Register ttos timer clockchip */
    rc = clockchip_register(cc);
    if (rc)
    {
        free(cc);
        KLOG_EMERG("fail at %s:%d", __FILE__, __LINE__);
        return -1;
    }

    sync_counter();

    irq = ttos_core_pic_irq_alloc(LOONGARCH_TIMER_PHYS_IRQ);

    /* Register irq handler for timer */
    rc = ttos_pic_irq_install(irq, generic_phys_timer_handler, cc, IRQ_SHARED, "PHYSICAL_TIMER");
    if (rc)
    {
        clockchip_unregister(cc);
        ttos_pic_irq_uninstall(irq, "PHYSICAL_TIMER");
        free(cc);
    }

    ttos_pic_irq_priority_set(irq, 0);
    ttos_pic_irq_unmask(irq);

    return 0;
}

/**
 * @brief 初始化时钟芯片
 *
 * @details 初始化通用定时器作为时钟事件设备
 *          用于提供高精度定时器和调度器时钟
 *
 * @param 无
 *
 * @return 成功返回0，失败返回-1
 *
 * @note 时钟事件设备用于生成定时器中断
 */
int arch_timer_clockchip_init(void)
{
    /* Get and Check generic timer frequency */
    generic_timer_get_freq();

    if (generic_timer_hz == 0)
    {
        return -1;
    }

    generic_timer_startup();

    return 0;
}
