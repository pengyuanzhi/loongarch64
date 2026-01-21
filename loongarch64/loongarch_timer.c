#include <arch_timer.h>
#include <barrier.h>
#include <time/ktime.h>
#include <commonUtils.h>
#include <cpuid.h>
#include <cpu.h>
#include <ttos_pic.h>
#include <ttos_time.h>
#include <inttypes.h>
#define KLOG_TAG "LOONGARCH Timer"
#include <klog.h>
/************************宏 定 义******************************/
#define LOONGARCH_TIMER_PHYS_IRQ (11)
/************************类型定义******************************/
static u64 cpu_clock_freq;
static long init_offset;
/************************外部声明******************************/
/************************前向声明******************************/
static s32 loongarch_timer_init (void);
static s32 loongarch_timer_enable (void);
static s32 loongarch_timer_disable (void);
static u64 loongarch_timer_count_get (void);
static s32 loongarch_timer_count_set (u64 count);
static u64 loongarch_timer_freq_get (void);
static u64 loongarch_timer_walltime_get (void);
static s32 loongarch_timer_timeout_set (u64 timeout);
/************************模块变量******************************/
static ttos_time_ops_t loongarch_timer_ops
    = { .time_name         = "LOONGARCH Stable Timer",
        .time_init         = loongarch_timer_init,
        .time_enable       = loongarch_timer_enable,
        .time_disable      = loongarch_timer_disable,
        .time_count_get    = loongarch_timer_count_get,
        .time_count_set    = loongarch_timer_count_set,
        .time_freq_get     = loongarch_timer_freq_get,
        .time_walltime_get = loongarch_timer_walltime_get,
        .time_timeout_set  = loongarch_timer_timeout_set };
/************************全局变量******************************/
/************************实   现*******************************/
/************************外部实现start******************************/
/************************外部实现end******************************/
int constant_set_state_oneshot(void)
{
	unsigned long timer_config;
	timer_config = csr_read64(LOONGARCH_CSR_TCFG);
	timer_config |= CSR_TCFG_EN;
	timer_config &= ~CSR_TCFG_PERIOD;
	csr_write64(timer_config, LOONGARCH_CSR_TCFG);
	return 0;
}
int constant_set_state_periodic(void)
{
	unsigned long period;
	unsigned long timer_config;
	period = cpu_clock_freq / HZ;
	timer_config = period & CSR_TCFG_VAL;
	timer_config |= (CSR_TCFG_PERIOD | CSR_TCFG_EN);
	csr_write64(timer_config, LOONGARCH_CSR_TCFG);
	return 0;
}
static int constant_set_state_shutdown(void)
{
	unsigned long timer_config;
	timer_config = csr_read64(LOONGARCH_CSR_TCFG);
	timer_config &= ~CSR_TCFG_EN;
	csr_write64(timer_config, LOONGARCH_CSR_TCFG);
	return 0;
}
void sync_counter(void)
{
	/* Ensure counter begin at 0 */
    init_offset = -(drdtime() - csr_read64(LOONGARCH_CSR_CNTC));
	csr_write64(init_offset, LOONGARCH_CSR_CNTC);
}
static void loongarch_timer_freq_init(void)
{
    cpu_clock_freq = calc_const_freq();
}
/**
 * @brief
 *    timer 中断初始化
 * @param[in] 无
 * @retval 无
 */
static void loongarch_timer_irq_init (void)
{
    ttos_pic_irq_unmask (LOONGARCH_TIMER_PHYS_IRQ);
}
/**
 * @brief
 *    获取timer freq
 * @param[in] 无
 * @retval  time freq
 */
static u64 loongarch_timer_freq_get (void)
{
    return calc_const_freq();
}
/**
 * @brief
 *    timer 初始化
 * @param[in] 无
 * @retval EIO 失败
 * @retval 0 成功
 */
static s32 loongarch_timer_init (void)
{
    /* timer freq 初始化 */
    loongarch_timer_freq_init();
    /* timer 中断初始化 */
    loongarch_timer_irq_init();
    if (is_bootcpu ())
    {
        KLOG_I ("loongarch timer freq:%" PRIu64, loongarch_timer_freq_get());
    }
}
/**
 * @brief
 *    使能timer
 * @param[in] 无
 * @retval 0 设置成功
 */
static s32 loongarch_timer_enable (void)
{
    /*使能timer*/
	unsigned long timer_config;
	timer_config = csr_read64(LOONGARCH_CSR_TCFG);
	timer_config |= CSR_TCFG_EN;
	csr_write64(timer_config, LOONGARCH_CSR_TCFG);
    return (0);
}
/**
 * @brief
 *    关闭timer
 * @param[in] 无
 * @retval 0 设置成功
 */
static s32 loongarch_timer_disable (void)
{
    /*disable timer*/
    constant_set_state_shutdown();
    return (0);
}
/**
 * @brief
 *    获取timer count
 * @param[in] 无
 * @retval  time count
 */
static u64 loongarch_timer_count_get (void)
{
    return drdtime();
}
/**
 * @brief
 *    设置timer count,count必须为4的整数倍
 * @param[in] count time count
 * @retval 0 设置成功
 */
static s32 loongarch_timer_count_set (u64 count)
{
	unsigned long timer_config;
	timer_config = (count & CSR_TCFG_VAL) ? (count & CSR_TCFG_VAL) : (1<<CSR_TCFG_VAL_SHIFT);
    /* 使能timer */
	timer_config |= CSR_TCFG_EN;
	csr_write64(timer_config, LOONGARCH_CSR_TCFG);
    return (0);
}
/**
 * @brief
 *    获取timer墙上时间
 * @param[in] 无
 * @retval 墙上时间，nanoseconds
 */
static u64 loongarch_timer_walltime_get (void)
{
    u64 freq, count;
    u64 seconds, nanoseconds, walltime;
    /*获取频率*/
    freq = loongarch_timer_freq_get ();
    /*获取timer count*/
    count = loongarch_timer_count_get ();
    /*计算walltime*/
    seconds     = count / freq;
    nanoseconds = ((count % freq) * NSEC_PER_SEC) / freq;
    walltime    = seconds * NSEC_PER_SEC + nanoseconds;
    return walltime;
}
/**
 * @brief
 *    设置timer超时时刻
 * @param[in] timeout 超时时刻，nanoseconds
 * @retval EIO 失败
 * @retval 0 成功
 */
static s32 loongarch_timer_timeout_set (u64 timeout)
{
    u64 freq, count;
    u64 seconds;
    /*获取频率*/
    freq = loongarch_timer_freq_get ();
    /*计算count*/
    seconds = timeout / NSEC_PER_SEC;
    count   = ((timeout % NSEC_PER_SEC) * freq) / NSEC_PER_SEC;
    count   = seconds * freq + count;
    if(count % 4)
    {
        KLOG_E("loongarch timer count %d,it must be an integer multiple of 4", count);
    }
    return loongarch_timer_count_set (count);
}
/**
 * @brief
 *    arm timer初始化，注册ttos_timer操作函数
 * @param[in] 无
 * @retval 0 初始化成功
 */
s32 arch_timer_pre_init (void)
{
    if (is_bootcpu ())
    {
        /*注册loongarch timer操作函数*/
        ttos_time_register (&loongarch_timer_ops);
    }
    return (0);
}
