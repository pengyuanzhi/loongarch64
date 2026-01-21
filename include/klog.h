/**
 * @file    klog.h
 * @brief   内核日志接口
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 提供内核日志输出接口，支持不同日志级别
 *
 * @note MISRA-C:2012 合规
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef KLOG_H
#define KLOG_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef KLOG_TAG
/**
 * @brief 日志标签默认值
 */
#define KLOG_TAG "NOTAG"
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

    /**
     * @brief 日志级别枚举
     */
    enum klog_level
    {
        KLOG_EMER = 0, /**< @brief 系统不可用 */
        KLOG_ALERT,    /**< @brief 必须立即采取措施 */
        KLOG_CRIT,     /**< @brief 严重情况 */
        KLOG_ERR,      /**< @brief 错误情况 */
        KLOG_WARN,     /**< @brief 警告情况 */
        KLOG_NOTICE,   /**< @brief 正常重要情况 */
        KLOG_INFO,     /**< @brief 信息性消息 */
        KLOG_DEBUG,    /**< @brief 调试级别消息 */
        KLOG_END,
        KLOG_MAX = 0xFFFFFFFFU,
    };

#ifndef KLOG_LEVEL
#ifdef CONFIG_LOG_LEVEL_DEBUG
#define KLOG_LEVEL KLOG_DEBUG
#elif defined(CONFIG_LOG_LEVEL_INFO)
#define KLOG_LEVEL KLOG_INFO
#elif defined(CONFIG_LOG_LEVEL_WARN)
#define KLOG_LEVEL KLOG_WARN
#elif defined(CONFIG_LOG_LEVEL_ERROR)
#define KLOG_LEVEL KLOG_ERR
#elif defined(CONFIG_LOG_LEVEL_EMERG)
#define KLOG_LEVEL KLOG_EMER
#else
#define KLOG_LEVEL KLOG_EMER
#endif
#endif

/**
 * @brief KLOG魔数（'klog'的ASCII）
 */
#define KLOG_MAGIC 0x6b6c6f67U

    /**
     * @brief 日志记录结构
     */
    struct klog_record
    {
        unsigned int magic; /**< @brief 魔数 */
        int klog_level;     /**< @brief 日志级别 */
        long ts_sec;        /**< @brief 时间戳（秒） */
        long ts_nsec;       /**< @brief 时间戳（纳秒） */
        unsigned int seq;   /**< @brief 序列号 */
        int buf_size;       /**< @brief 缓冲区大小 */
    };

    /**
     * @brief 打印内核日志
     *
     * @param log_level 日志级别
     * @param is_buf    是否使用缓冲区
     * @param tag       日志标签
     * @param fmt       格式化字符串
     * @param ...       可变参数
     */
    void klog_print(int log_level, int is_buf, const char *tag, const char *fmt, ...);

/**
 * @brief 内部日志输出宏
 */
#define KLOG_X(level, ...)                           \
    do                                               \
    {                                                \
        klog_print(level, 1, KLOG_TAG, __VA_ARGS__); \
    } while (0)

/**
 * @brief DEBUG级别日志宏
 */
#define KLOG_D(arg...) KLOG_X(KLOG_DEBUG, arg)

/**
 * @brief INFO级别日志宏
 */
#define KLOG_I(arg...) KLOG_X(KLOG_INFO, arg)

/**
 * @brief WARN级别日志宏
 */
#define KLOG_W(arg...) KLOG_X(KLOG_WARN, arg)

/**
 * @brief ERROR级别日志宏
 */
#define KLOG_E(arg...) KLOG_X(KLOG_ERR, arg)

/**
 * @brief EMERG级别日志宏
 */
#define KLOG_EMERG(arg...) KLOG_X(KLOG_EMER, arg)

    /**
     * @brief 从循环缓冲区读取日志
     *
     * @param buf 缓冲区指针
     * @param pos 起始位置
     * @param len 读取长度
     *
     * @return 实际读取的字节数
     */
    int klog_read_circbuf(char *buf, int pos, int len);

    /**
     * @brief 读取日志消息
     *
     * @param buffer 缓冲区
     * @param pos    位置指针（输入/输出）
     * @param buflen 缓冲区长度
     * @param block  是否阻塞
     *
     * @return 实际读取的字节数
     */
    int klog_read_msg(char *buffer, unsigned int *pos, unsigned int buflen, bool block);

    /**
     * @brief 从klogctl读取日志
     *
     * @param buffer 缓冲区
     * @param buflen 缓冲区长度
     * @param clear  是否清除日志
     *
     * @return 实际读取的字节数
     */
    int klog_read_from_klogctl(char *buffer, unsigned int buflen, bool clear);

    /**
     * @brief 获取日志缓冲区大小
     *
     * @return 日志缓冲区大小（字节）
     */
    size_t klog_get_size(void);

    /**
     * @brief 设置日志级别
     *
     * @param level 日志级别
     *
     * @return 0表示成功，-1表示失败
     */
    int klog_level_set(int level);

    /**
     * @brief 获取当前日志级别
     *
     * @return 当前日志级别
     */
    int klog_level_get(void);

    /**
     * @brief 清除日志缓冲区
     *
     * @return 0表示成功，-1表示失败
     */
    int klog_clear(void);

#define DUMP_PREFIX_NONE 0U    /**< @brief 不加任何前缀 */
#define DUMP_PREFIX_ADDRESS 1U /**< @brief 每行前缀加地址（buf中相对地址） */
#define DUMP_PREFIX_OFFSET 2U  /**< @brief 每行前缀加偏移量 */

    /**
     * @brief 打印十六进制转储
     *
     * @param level      日志级别
     * @param prefix_str 行前缀字符串
     * @param prefix_type 前缀类型
     * @param rowsize    每行字节数
     * @param groupsize  每组字节数
     * @param buf        数据缓冲区
     * @param len        数据长度
     * @param ascii      是否显示ASCII
     */
    void log_print_hex_dump(int level, const char *prefix_str, int prefix_type, int rowsize, int groupsize,
                            const void *buf, size_t len, bool ascii);

/* 兼容Linux */
#define print_hex_dump(a, b, c, d, e, f, g) log_print_hex_dump(KLOG_DEBUG, a, b, c, d, e, f, g)

#define KLOG_HEX_D(a, b, c, d, e, f, g) log_print_hex_dump(KLOG_DEBUG, a, b, c, d, e, f, g)

#define KLOG_HEX_I(a, b, c, d, e, f, g) log_print_hex_dump(KLOG_INFO, a, b, c, d, e, f, g)

#define KLOG_HEX_W(a, b, c, d, e, f, g) log_print_hex_dump(KLOG_WARN, a, b, c, d, e, f, g)

#define KLOG_HEX_E(a, b, c, d, e, f, g) log_print_hex_dump(KLOG_ERR, a, b, c, d, e, f, g)

#define KLOG_HEX_EMERG(a, b, c, d, e, f, g) log_print_hex_dump(KLOG_EMERG, a, b, c, d, e, f, g)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KLOG_H */
