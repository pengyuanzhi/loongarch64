/**
 * @file    pe.h
 * @brief   PE（Portable Executable）格式定义
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件定义PE/COFF文件格式相关常量和结构
 *          - PE魔数定义
 *          - 机器类型定义
 *          - 文件标志定义
 *          - 节区属性定义
 *          - PE头结构定义
 *          - 重定位类型定义
 *
 * @note MISRA-C:2012 合规
 * @note PE格式是Windows可执行文件格式
 * @note 支持LoongArch32和LoongArch64机器类型
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

#ifndef _PE_H
#define _PE_H

/************************头 文 件******************************/
#include <system/types.h>

/************************宏 定 义******************************/

/**
 * @defgroup PEMagic PE魔数定义
 * @{
 */

/**
 * @brief Linux EFI存根版本定义
 *
 * @details 从v3.0开始，主版本字段应解释为内核EFI stub支持的特性位掩码：
 *          - 0x1: 从LINUX_EFI_INITRD_MEDIA_GUID设备路径加载initrd
 *          - 0x2: 使用initrd=命令行选项加载initrd
 */
#define LINUX_EFISTUB_MAJOR_VERSION 0x3U
#define LINUX_EFISTUB_MINOR_VERSION 0x0U

/**
 * @brief Linux PE魔数
 *
 * @details 出现在EFI可引导Linux内核镜像MS-DOS头的0x38偏移处
 */
#define LINUX_PE_MAGIC 0x818223cdU

/**
 * @brief MS-DOS魔数
 */
#define MZ_MAGIC 0x5a4dU /**< "MZ" */

/**
 * @brief PE魔数
 */
#define PE_MAGIC 0x00004550U /**< "PE\0\0" */

/**
 * @brief PE可选头魔数
 */
#define PE_OPT_MAGIC_PE32 0x010bU     /**< PE32 */
#define PE_OPT_MAGIC_PE32_ROM 0x0107U /**< PE32 ROM */
#define PE_OPT_MAGIC_PE32PLUS 0x020bU /**< PE32+ */

/** @} */

/**
 * @defgroup PEMachineTypes 机器类型定义
 * @{
 */

#define IMAGE_FILE_MACHINE_UNKNOWN 0x0000U     /**< 未知机器 */
#define IMAGE_FILE_MACHINE_AM33 0x01d3U        /**< AM33 */
#define IMAGE_FILE_MACHINE_AMD64 0x8664U       /**< AMD64/x86-64 */
#define IMAGE_FILE_MACHINE_ARM 0x01c0U         /**< ARM小端 */
#define IMAGE_FILE_MACHINE_ARMV7 0x01c4U       /**< ARMv7 Thumb */
#define IMAGE_FILE_MACHINE_ARM64 0xaa64U       /**< ARM64/AArch64 */
#define IMAGE_FILE_MACHINE_EBC 0x0ebcU         /**< EBC字节码 */
#define IMAGE_FILE_MACHINE_I386 0x014cU        /**< Intel 386及后续 */
#define IMAGE_FILE_MACHINE_IA64 0x0200U        /**< Intel IA64 */
#define IMAGE_FILE_MACHINE_M32R 0x9041U        /**< Mitsubishi M32R */
#define IMAGE_FILE_MACHINE_MIPS16 0x0266U      /**< MIPS16 */
#define IMAGE_FILE_MACHINE_MIPSFPU 0x0366U     /**< MIPS with FPU */
#define IMAGE_FILE_MACHINE_MIPSFPU16 0x0466U   /**< MIPS16 with FPU */
#define IMAGE_FILE_MACHINE_POWERPC 0x01f0U     /**< PowerPC小端 */
#define IMAGE_FILE_MACHINE_POWERPCFP 0x01f1U   /**< PowerPC with FPU */
#define IMAGE_FILE_MACHINE_R4000 0x0166U       /**< MIPS little endian */
#define IMAGE_FILE_MACHINE_RISCV32 0x5032U     /**< RISC-V 32位 */
#define IMAGE_FILE_MACHINE_RISCV64 0x5064U     /**< RISC-V 64位 */
#define IMAGE_FILE_MACHINE_RISCV128 0x5128U    /**< RISC-V 128位 */
#define IMAGE_FILE_MACHINE_SH3 0x01a2U         /**< Hitachi SH3 */
#define IMAGE_FILE_MACHINE_SH3DSP 0x01a3U      /**< Hitachi SH3 DSP */
#define IMAGE_FILE_MACHINE_SH3E 0x01a4U        /**< Hitachi SH3E */
#define IMAGE_FILE_MACHINE_SH4 0x01a6U         /**< Hitachi SH4 */
#define IMAGE_FILE_MACHINE_SH5 0x01a8U         /**< Hitachi SH5 */
#define IMAGE_FILE_MACHINE_THUMB 0x01c2U       /**< Thumb */
#define IMAGE_FILE_MACHINE_WCEMIPSV2 0x0169U   /**< MIPS WCE v2 */
#define IMAGE_FILE_MACHINE_LOONGARCH32 0x6232U /**< LoongArch 32位 */
#define IMAGE_FILE_MACHINE_LOONGARCH64 0x6264U /**< LoongArch 64位 */

/** @} */

/**
 * @defgroup PEFileFlags PE文件标志
 * @{
 */

#define IMAGE_FILE_RELOCS_STRIPPED 0x0001U         /**< 重定位被剥离 */
#define IMAGE_FILE_EXECUTABLE_IMAGE 0x0002U        /**< 可执行文件 */
#define IMAGE_FILE_LINE_NUMS_STRIPPED 0x0004U      /**< 行号被剥离 */
#define IMAGE_FILE_LOCAL_SYMS_STRIPPED 0x0008U     /**< 本地符号被剥离 */
#define IMAGE_FILE_AGGRESSIVE_WS_TRIM 0x0010U      /**< 积极工作集修剪 */
#define IMAGE_FILE_LARGE_ADDRESS_AWARE 0x0020U     /**< 大地址感知 */
#define IMAGE_FILE_16BIT_MACHINE 0x0040U           /**< 16位机器 */
#define IMAGE_FILE_BYTES_REVERSED_LO 0x0080U       /**< 字节序反转（低位） */
#define IMAGE_FILE_32BIT_MACHINE 0x0100U           /**< 32位机器 */
#define IMAGE_FILE_DEBUG_STRIPPED 0x0200U          /**< 调试信息被剥离 */
#define IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP 0x0400U /**< 可移动介质运行 */
#define IMAGE_FILE_NET_RUN_FROM_SWAP 0x0800U       /**< 网络运行 */
#define IMAGE_FILE_SYSTEM 0x1000U                  /**< 系统文件 */
#define IMAGE_FILE_DLL 0x2000U                     /**< DLL文件 */
#define IMAGE_FILE_UP_SYSTEM_ONLY 0x4000U          /**< 单处理器系统 */
#define IMAGE_FILE_BYTES_REVERSED_HI 0x8000U       /**< 字节序反转（高位） */

#define IMAGE_FILE_OPT_ROM_MAGIC 0x107U       /**< ROM可选头魔数 */
#define IMAGE_FILE_OPT_PE32_MAGIC 0x10bU      /**< PE32可选头魔数 */
#define IMAGE_FILE_OPT_PE32_PLUS_MAGIC 0x20bU /**< PE32+可选头魔数 */

/** @} */

/**
 * @defgroup PESubsystem PE子系统类型
 * @{
 */

#define IMAGE_SUBSYSTEM_UNKNOWN 0U                  /**< 未知子系统 */
#define IMAGE_SUBSYSTEM_NATIVE 1U                   /**< 设备驱动和原生Windows */
#define IMAGE_SUBSYSTEM_WINDOWS_GUI 2U              /**< Windows GUI */
#define IMAGE_SUBSYSTEM_WINDOWS_CUI 3U              /**< Windows CUI */
#define IMAGE_SUBSYSTEM_POSIX_CUI 7U                /**< POSIX CUI */
#define IMAGE_SUBSYSTEM_WINDOWS_CE_GUI 9U           /**< Windows CE GUI */
#define IMAGE_SUBSYSTEM_EFI_APPLICATION 10U         /**< EFI应用程序 */
#define IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER 11U /**< EFI启动服务驱动 */
#define IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER 12U      /**< EFI运行时驱动 */
#define IMAGE_SUBSYSTEM_EFI_ROM_IMAGE 13U           /**< EFI ROM镜像 */
#define IMAGE_SUBSYSTEM_XBOX 14U                    /**< XBOX */

/** @} */

/**
 * @defgroup PEDLLCharacteristics DLL特性
 * @{
 */

#define IMAGE_DLL_CHARACTERISTICS_DYNAMIC_BASE 0x0040U         /**< ASLR */
#define IMAGE_DLL_CHARACTERISTICS_FORCE_INTEGRITY 0x0080U      /**< 强制完整性检查 */
#define IMAGE_DLL_CHARACTERISTICS_NX_COMPAT 0x0100U            /**< NX兼容 */
#define IMAGE_DLLCHARACTERISTICS_NO_ISOLATION 0x0200U          /**< 无隔离 */
#define IMAGE_DLLCHARACTERISTICS_NO_SEH 0x0400U                /**< 无SEH */
#define IMAGE_DLLCHARACTERISTICS_NO_BIND 0x0800U               /**< 无绑定 */
#define IMAGE_DLLCHARACTERISTICS_WDM_DRIVER 0x2000U            /**< WDM驱动 */
#define IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE 0x8000U /**< 终端服务器感知 */
#define IMAGE_DLLCHARACTERISTICS_EX_CET_COMPAT 0x0001U         /**< CET兼容 */
#define IMAGE_DLLCHARACTERISTICS_EX_FORWARD_CFI_COMPAT 0x0040U /**< 前向CFI兼容 */

/** @} */

/**
 * @defgroup PESectionCharacteristics 节区特性
 * @{
 */

#define IMAGE_SCN_RESERVED_0 0x00000001U             /**< 保留 */
#define IMAGE_SCN_RESERVED_1 0x00000002U             /**< 保留 */
#define IMAGE_SCN_RESERVED_2 0x00000004U             /**< 保留 */
#define IMAGE_SCN_TYPE_NO_PAD 0x00000008U            /**< 不填充 */
#define IMAGE_SCN_RESERVED_3 0x00000010U             /**< 保留 */
#define IMAGE_SCN_CNT_CODE 0x00000020U               /**< 包含代码 */
#define IMAGE_SCN_CNT_INITIALIZED_DATA 0x00000040U   /**< 包含已初始化数据 */
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA 0x00000080U /**< 包含未初始化数据 */
#define IMAGE_SCN_LNK_OTHER 0x00000100U              /**< 保留 */
#define IMAGE_SCN_LNK_INFO 0x00000200U               /**< .drectve注释 */
#define IMAGE_SCN_RESERVED_4 0x00000400U             /**< 保留 */
#define IMAGE_SCN_LNK_REMOVE 0x00000800U             /**< 被移除 */
#define IMAGE_SCN_LNK_COMDAT 0x00001000U             /**< COMDAT数据 */
#define IMAGE_SCN_RESERVED_5 0x00002000U             /**< 保留 */
#define IMAGE_SCN_RESERVED_6 0x00004000U             /**< 保留 */
#define IMAGE_SCN_GPREL 0x00008000U                  /**< 全局指针引用 */
#define IMAGE_SCN_MEM_PURGEABLE 0x00010000U          /**< 可清除 */
#define IMAGE_SCN_16BIT 0x00020000U                  /**< 16位 */
#define IMAGE_SCN_LOCKED 0x00040000U                 /**< 锁定 */
#define IMAGE_SCN_PRELOAD 0x00080000U                /**< 预加载 */
#define IMAGE_SCN_ALIGN_1BYTES 0x00100000U           /**< 1字节对齐 */
#define IMAGE_SCN_ALIGN_2BYTES 0x00200000U           /**< 2字节对齐 */
#define IMAGE_SCN_ALIGN_4BYTES 0x00300000U           /**< 4字节对齐 */
#define IMAGE_SCN_ALIGN_8BYTES 0x00400000U           /**< 8字节对齐 */
#define IMAGE_SCN_ALIGN_16BYTES 0x00500000U          /**< 16字节对齐 */
#define IMAGE_SCN_ALIGN_32BYTES 0x00600000U          /**< 32字节对齐 */
#define IMAGE_SCN_ALIGN_64BYTES 0x00700000U          /**< 64字节对齐 */
#define IMAGE_SCN_ALIGN_128BYTES 0x00800000U         /**< 128字节对齐 */
#define IMAGE_SCN_ALIGN_256BYTES 0x00900000U         /**< 256字节对齐 */
#define IMAGE_SCN_ALIGN_512BYTES 0x00a00000U         /**< 512字节对齐 */
#define IMAGE_SCN_ALIGN_1024BYTES 0x00b00000U        /**< 1024字节对齐 */
#define IMAGE_SCN_ALIGN_2048BYTES 0x00c00000U        /**< 2048字节对齐 */
#define IMAGE_SCN_ALIGN_4096BYTES 0x00d00000U        /**< 4096字节对齐 */
#define IMAGE_SCN_ALIGN_8192BYTES 0x00e00000U        /**< 8192字节对齐 */
#define IMAGE_SCN_LNK_NRELOC_OVFL 0x01000000U        /**< 扩展重定位 */
#define IMAGE_SCN_MEM_DISCARDABLE 0x02000000U        /**< 可丢弃 */
#define IMAGE_SCN_MEM_NOT_CACHED 0x04000000U         /**< 不可缓存 */
#define IMAGE_SCN_MEM_NOT_PAGED 0x08000000U          /**< 不可分页 */
#define IMAGE_SCN_MEM_SHARED 0x10000000U             /**< 可共享 */
#define IMAGE_SCN_MEM_EXECUTE 0x20000000U            /**< 可执行 */
#define IMAGE_SCN_MEM_READ 0x40000000U               /**< 可读 */
#define IMAGE_SCN_MEM_WRITE 0x80000000U              /**< 可写 */

/** @} */

/**
 * @defgroup PEDebugTypes 调试信息类型
 * @{
 */

#define IMAGE_DEBUG_TYPE_CODEVIEW 2U               /**< CodeView调试信息 */
#define IMAGE_DEBUG_TYPE_EX_DLLCHARACTERISTICS 20U /**< 扩展DLL特性 */

/** @} */

/************************类型定义******************************/

#ifndef ASM_USE

/**
 * @defgroup PEStructures PE结构体定义
 * @{
 */

/**
 * @brief MS-DOS头结构
 */
struct mz_hdr
{
    uint16_t magic;              /**< MZ_MAGIC */
    uint16_t lbsize;             /**< 最后一块大小 */
    uint16_t blocks;             /**< 文件页数 */
    uint16_t relocs;             /**< 重定位项 */
    uint16_t hdrsize;            /**< 头大小（以段为单位） */
    uint16_t min_extra_pps;      /**< .bss最小额外段 */
    uint16_t max_extra_pps;      /**< 运行时最大额外段 */
    uint16_t ss;                 /**< 相对栈段 */
    uint16_t sp;                 /**< 初始%sp寄存器 */
    uint16_t checksum;           /**< 字校验和 */
    uint16_t ip;                 /**< 初始%ip寄存器 */
    uint16_t cs;                 /**< 相对加载段的%cs */
    uint16_t reloc_table_offset; /**< 第一个重定位偏移 */
    uint16_t overlay_num;        /**< 覆盖号，设置为0 */
    uint16_t reserved0[4U];      /**< 保留 */
    uint16_t oem_id;             /**< OEM标识符 */
    uint16_t oem_info;           /**< OEM特定信息 */
    uint16_t reserved1[10U];     /**< 保留 */
    uint32_t peaddr;             /**< PE头地址 */
    char message[];              /**< 要打印的消息 */
};

/**
 * @brief MS-DOS重定位结构
 */
struct mz_reloc
{
    uint16_t offset;  /**< 偏移 */
    uint16_t segment; /**< 段 */
};

/**
 * @brief PE头结构
 */
struct pe_hdr
{
    uint32_t magic;        /**< PE魔数 */
    uint16_t machine;      /**< 机器类型 */
    uint16_t sections;     /**< 节数量 */
    uint32_t timestamp;    /**< 时间戳 */
    uint32_t symbol_table; /**< 符号表偏移 */
    uint32_t symbols;      /**< 符号数量 */
    uint16_t opt_hdr_size; /**< 可选头大小 */
    uint16_t flags;        /**< 标志 */
};

/**
 * @brief PE32可选头结构
 */
struct pe32_opt_hdr
{
    /* 标准字段 */
    uint16_t magic;       /**< 文件类型 */
    uint8_t ld_major;     /**< 链接器主版本 */
    uint8_t ld_minor;     /**< 链接器次版本 */
    uint32_t text_size;   /**< 代码节大小 */
    uint32_t data_size;   /**< 数据节大小 */
    uint32_t bss_size;    /**< BSS节大小 */
    uint32_t entry_point; /**< 入口点文件偏移 */
    uint32_t code_base;   /**< 代码相对地址 */
    uint32_t data_base;   /**< 数据相对地址 */
    /* Windows特定字段 */
    uint32_t image_base;     /**< 首选加载地址 */
    uint32_t section_align;  /**< 内存对齐 */
    uint32_t file_align;     /**< 文件对齐 */
    uint16_t os_major;       /**< OS主版本 */
    uint16_t os_minor;       /**< OS次版本 */
    uint16_t image_major;    /**< 镜像主版本 */
    uint16_t image_minor;    /**< 镜像次版本 */
    uint16_t subsys_major;   /**< 子系统主版本 */
    uint16_t subsys_minor;   /**< 子系统次版本 */
    uint32_t win32_version;  /**< 保留，必须为0 */
    uint32_t image_size;     /**< 镜像大小 */
    uint32_t header_size;    /**< 头大小 */
    uint32_t csum;           /**< 校验和 */
    uint16_t subsys;         /**< 子系统 */
    uint16_t dll_flags;      /**< DLL标志 */
    uint32_t stack_size_req; /**< 请求栈大小 */
    uint32_t stack_size;     /**< 栈大小 */
    uint32_t heap_size_req;  /**< 请求堆大小 */
    uint32_t heap_size;      /**< 堆大小 */
    uint32_t loader_flags;   /**< 保留，必须为0 */
    uint32_t data_dirs;      /**< 数据目录项数 */
};

/**
 * @brief PE32+可选头结构
 */
struct pe32plus_opt_hdr
{
    uint16_t magic;       /**< 文件类型 */
    uint8_t ld_major;     /**< 链接器主版本 */
    uint8_t ld_minor;     /**< 链接器次版本 */
    uint32_t text_size;   /**< 代码节大小 */
    uint32_t data_size;   /**< 数据节大小 */
    uint32_t bss_size;    /**< BSS节大小 */
    uint32_t entry_point; /**< 入口点文件偏移 */
    uint32_t code_base;   /**< 代码相对地址 */
    /* Windows特定字段 */
    uint64_t image_base;     /**< 首选加载地址 */
    uint32_t section_align;  /**< 内存对齐 */
    uint32_t file_align;     /**< 文件对齐 */
    uint16_t os_major;       /**< OS主版本 */
    uint16_t os_minor;       /**< OS次版本 */
    uint16_t image_major;    /**< 镜像主版本 */
    uint16_t image_minor;    /**< 镜像次版本 */
    uint16_t subsys_major;   /**< 子系统主版本 */
    uint16_t subsys_minor;   /**< 子系统次版本 */
    uint32_t win32_version;  /**< 保留，必须为0 */
    uint32_t image_size;     /**< 镜像大小 */
    uint32_t header_size;    /**< 头大小 */
    uint32_t csum;           /**< 校验和 */
    uint16_t subsys;         /**< 子系统 */
    uint16_t dll_flags;      /**< DLL标志 */
    uint64_t stack_size_req; /**< 请求栈大小 */
    uint64_t stack_size;     /**< 栈大小 */
    uint64_t heap_size_req;  /**< 请求堆大小 */
    uint64_t heap_size;      /**< 堆大小 */
    uint32_t loader_flags;   /**< 保留，必须为0 */
    uint32_t data_dirs;      /**< 数据目录项数 */
};

/**
 * @brief 数据目录项结构
 */
struct data_dirent
{
    uint32_t virtual_address; /**< 相对加载地址 */
    uint32_t size;            /**< 大小 */
};

/**
 * @brief 数据目录结构
 */
struct data_directory
{
    struct data_dirent exports;          /**< .edata */
    struct data_dirent imports;          /**< .idata */
    struct data_dirent resources;        /**< .rsrc */
    struct data_dirent exceptions;       /**< .pdata */
    struct data_dirent certs;            /**< 证书 */
    struct data_dirent base_relocations; /**< .reloc */
    struct data_dirent debug;            /**< .debug */
    struct data_dirent arch;             /**< 保留 */
    struct data_dirent global_ptr;       /**< 全局指针寄存器，大小=0 */
    struct data_dirent tls;              /**< .tls */
    struct data_dirent load_config;      /**< 加载配置结构 */
    struct data_dirent bound_imports;    /**< 绑定导入 */
    struct data_dirent import_addrs;     /**< 导入地址表 */
    struct data_dirent delay_imports;    /**< 延迟加载导入表 */
    struct data_dirent clr_runtime_hdr;  /**< .cor（仅对象） */
    struct data_dirent reserved;         /**< 保留 */
};

/**
 * @brief 节头结构
 */
struct section_header
{
    char name[8U];            /**< 名称或"/12\0"字符串表偏移 */
    uint32_t virtual_size;    /**< 加载到RAM的节大小 */
    uint32_t virtual_address; /**< 相对虚拟地址 */
    uint32_t raw_data_size;   /**< 节大小 */
    uint32_t data_addr;       /**< 第一页文件指针 */
    uint32_t relocs;          /**< 重定位项文件指针 */
    uint32_t line_numbers;    /**< 行号！ */
    uint16_t num_relocs;      /**< 重定位数量 */
    uint16_t num_lin_numbers; /**< 行号数量 */
    uint32_t flags;           /**< 标志 */
};

/** @} */

/**
 * @defgroup PERelocationTypes 重定位类型
 * @{
 */

/**
 * @brief x64 COFF重定位类型枚举
 */
enum x64_coff_reloc_type
{
    IMAGE_REL_AMD64_ABSOLUTE = 0, /**< 绝对重定位 */
    IMAGE_REL_AMD64_ADDR64,       /**< 64位地址 */
    IMAGE_REL_AMD64_ADDR32,       /**< 32位地址 */
    IMAGE_REL_AMD64_ADDR32N,      /**< 32位地址-相对32位 */
    IMAGE_REL_AMD64_REL32,        /**< 32位相对地址 */
    IMAGE_REL_AMD64_REL32_1,      /**< 32位相对地址-1 */
    IMAGE_REL_AMD64_REL32_2,      /**< 32位相对地址-2 */
    IMAGE_REL_AMD64_REL32_3,      /**< 32位相对地址-3 */
    IMAGE_REL_AMD64_REL32_4,      /**< 32位相对地址-4 */
    IMAGE_REL_AMD64_REL32_5,      /**< 32位相对地址-5 */
    IMAGE_REL_AMD64_SECTION,      /**< 节索引 */
    IMAGE_REL_AMD64_SECREL,       /**< 32位节相对 */
    IMAGE_REL_AMD64_SECREL7,      /**< 7位节相对 */
    IMAGE_REL_AMD64_TOKEN,        /**< CLR令牌 */
    IMAGE_REL_AMD64_SREL32,       /**< 32位相对节 */
    IMAGE_REL_AMD64_PAIR,         /**< 32位对 */
    IMAGE_REL_AMD64_SSPAN32,      /**< 32位跨节相对 */
};

/**
 * @brief ARM COFF重定位类型枚举
 */
enum arm_coff_reloc_type
{
    IMAGE_REL_ARM_ABSOLUTE, /**< 绝对重定位 */
    IMAGE_REL_ARM_ADDR32,   /**< 32位地址 */
    IMAGE_REL_ARM_ADDR32N,  /**< 32位地址-相对32位 */
    IMAGE_REL_ARM_BRANCH2,  /**< ARM分支（2条指令） */
    IMAGE_REL_ARM_BRANCH1,  /**< ARM分支（1条指令） */
    IMAGE_REL_ARM_SECTION,  /**< 节索引 */
    IMAGE_REL_ARM_SECREL,   /**< 32位节相对 */
};

/**
 * @brief SH3/SH4 COFF重定位类型枚举
 */
enum sh_coff_reloc_type
{
    IMAGE_REL_SH3_ABSOLUTE,        /**< 绝对重定位 */
    IMAGE_REL_SH3_DIRECT16,        /**< 16位直接 */
    IMAGE_REL_SH3_DIRECT32,        /**< 32位直接 */
    IMAGE_REL_SH3_DIRECT8,         /**< 8位直接 */
    IMAGE_REL_SH3_DIRECT8_WORD,    /**< 8位直接（字） */
    IMAGE_REL_SH3_DIRECT8_LONG,    /**< 8位直接（长） */
    IMAGE_REL_SH3_DIRECT4,         /**< 4位直接 */
    IMAGE_REL_SH3_DIRECT4_WORD,    /**< 4位直接（字） */
    IMAGE_REL_SH3_DIRECT4_LONG,    /**< 4位直接（长） */
    IMAGE_REL_SH3_PCREL8_WORD,     /**< 8位PC相对（字） */
    IMAGE_REL_SH3_PCREL8_LONG,     /**< 8位PC相对（长） */
    IMAGE_REL_SH3_PCREL12_WORD,    /**< 12位PC相对（字） */
    IMAGE_REL_SH3_STARTOF_SECTION, /**< 节起始 */
    IMAGE_REL_SH3_SIZEOF_SECTION,  /**< 节大小 */
    IMAGE_REL_SH3_SECTION,         /**< 节索引 */
    IMAGE_REL_SH3_SECREL,          /**< 32位节相对 */
    IMAGE_REL_SH3_DIRECT32_NB,     /**< 32位直接（无分支预测） */
    IMAGE_REL_SH3_GPREL4_LONG,     /**< 4位GP相对（长） */
    IMAGE_REL_SH3_TOKEN,           /**< 令牌 */
    IMAGE_REL_SHM_PCRELPT,         /**< PC相对（页表） */
    IMAGE_REL_SHM_REFLO,           /**< 引用低位 */
    IMAGE_REL_SHM_REFHALF,         /**< 引用半字 */
    IMAGE_REL_SHM_RELLO,           /**< 相对低位 */
    IMAGE_REL_SHM_RELHALF,         /**< 相对半字 */
    IMAGE_REL_SHM_PAIR,            /**< 对 */
    IMAGE_REL_SHM_NOMODE,          /**< 无模式 */
};

/**
 * @brief PowerPC COFF重定位类型枚举
 */
enum ppc_coff_reloc_type
{
    IMAGE_REL_PPC_ABSOLUTE, /**< 绝对重定位 */
    IMAGE_REL_PPC_ADDR64,   /**< 64位地址 */
    IMAGE_REL_PPC_ADDR32,   /**< 32位地址 */
    IMAGE_REL_PPC_ADDR24,   /**< 24位地址 */
    IMAGE_REL_PPC_ADDR16,   /**< 16位地址 */
    IMAGE_REL_PPC_ADDR14,   /**< 14位地址 */
    IMAGE_REL_PPC_REL24,    /**< 24位相对 */
    IMAGE_REL_PPC_REL14,    /**< 14位相对 */
    IMAGE_REL_PPC_ADDR32N,  /**< 32位地址-相对32位 */
    IMAGE_REL_PPC_SECREL,   /**< 32位节相对 */
    IMAGE_REL_PPC_SECTION,  /**< 节索引 */
    IMAGE_REL_PPC_SECREL16, /**< 16位节相对 */
    IMAGE_REL_PPC_REFHI,    /**< 引用高位 */
    IMAGE_REL_PPC_REFLO,    /**< 引用低位 */
    IMAGE_REL_PPC_PAIR,     /**< 对 */
    IMAGE_REL_PPC_SECRELLO, /**< 低位节相对 */
    IMAGE_REL_PPC_GPREL,    /**< GP相对 */
    IMAGE_REL_PPC_TOKEN,    /**< 令牌 */
};

/**
 * @brief x86 COFF重定位类型枚举
 */
enum x86_coff_reloc_type
{
    IMAGE_REL_I386_ABSOLUTE, /**< 绝对重定位 */
    IMAGE_REL_I386_DIR16,    /**< 16位直接 */
    IMAGE_REL_I386_REL16,    /**< 16位相对 */
    IMAGE_REL_I386_DIR32,    /**< 32位直接 */
    IMAGE_REL_I386_DIR32NB,  /**< 32位直接（无分支预测） */
    IMAGE_REL_I386_SEG12,    /**< 12位段 */
    IMAGE_REL_I386_SECTION,  /**< 节索引 */
    IMAGE_REL_I386_SECREL,   /**< 32位节相对 */
    IMAGE_REL_I386_TOKEN,    /**< 令牌 */
    IMAGE_REL_I386_SECREL7,  /**< 7位节相对 */
    IMAGE_REL_I386_REL32,    /**< 32位相对 */
};

/**
 * @brief IA64 COFF重定位类型枚举
 */
enum ia64_coff_reloc_type
{
    IMAGE_REL_IA64_ABSOLUTE,  /**< 绝对重定位 */
    IMAGE_REL_IA64_IMM14,     /**< 14位立即数 */
    IMAGE_REL_IA64_IMM22,     /**< 22位立即数 */
    IMAGE_REL_IA64_IMM64,     /**< 64位立即数 */
    IMAGE_REL_IA64_DIR32,     /**< 32位直接 */
    IMAGE_REL_IA64_DIR64,     /**< 64位直接 */
    IMAGE_REL_IA64_PCREL21B,  /**< 21位PC相对（分支） */
    IMAGE_REL_IA64_PCREL21M,  /**< 21位PC相对（调用） */
    IMAGE_REL_IA64_PCREL21F,  /**< 21位PC相对（函数） */
    IMAGE_REL_IA64_GPREL22,   /**< 22位GP相对 */
    IMAGE_REL_IA64_LTOFF22,   /**< 22位链接时偏移 */
    IMAGE_REL_IA64_SECTION,   /**< 节索引 */
    IMAGE_REL_IA64_SECREL22,  /**< 22位节相对 */
    IMAGE_REL_IA64_SECREL64I, /**< 64位节相对（内部） */
    IMAGE_REL_IA64_SECREL32,  /**< 32位节相对 */
    IMAGE_REL_IA64_DIR32NB,   /**< 32位直接（无分支预测） */
    IMAGE_REL_IA64_SREL14,    /**< 14位相对 */
    IMAGE_REL_IA64_SREL22,    /**< 22位相对 */
    IMAGE_REL_IA64_SREL32,    /**< 32位相对 */
    IMAGE_REL_IA64_UREL32,    /**< 32位未完成相对 */
    IMAGE_REL_IA64_PCREL60X,  /**< 60位PC相对（扩展） */
    IMAGE_REL_IA64_PCREL60B,  /**< 60位PC相对（分支） */
    IMAGE_REL_IA64_PCREL60F,  /**< 60位PC相对（函数） */
    IMAGE_REL_IA64_PCREL60I,  /**< 60位PC相对（内部） */
    IMAGE_REL_IA64_PCREL60M,  /**< 60位PC相对（调用） */
    IMAGE_REL_IA64_IMMGPREL6, /**< 6位立即数GP相对 */
    IMAGE_REL_IA64_TOKEN,     /**< 令牌 */
    IMAGE_REL_IA64_GPREL32,   /**< 32位GP相对 */
    IMAGE_REL_IA64_ADDEND,    /**< 加数 */
};

/**
 * @brief COFF重定位结构
 */
struct coff_reloc
{
    uint32_t virtual_address;    /**< 虚拟地址 */
    uint32_t symbol_table_index; /**< 符号表索引 */
    union
    {
        enum x64_coff_reloc_type x64_type;
        enum arm_coff_reloc_type arm_type;
        enum sh_coff_reloc_type sh_type;
        enum ppc_coff_reloc_type ppc_type;
        enum x86_coff_reloc_type x86_type;
        enum ia64_coff_reloc_type ia64_type;
        uint16_t data;
    };
};

/** @} */

/**
 * @defgroup PECertificates PE证书定义
 * @{
 */

/**
 * @brief WIN证书类型
 */
#define WIN_CERT_TYPE_PKCS_SIGNED_DATA 0x0002U /**< PKCS签名数据 */
#define WIN_CERT_TYPE_EFI_OKCS115 0x0ef0U      /**< EFI OKCS115 */
#define WIN_CERT_TYPE_EFI_GUID 0x0ef1U         /**< EFI GUID */

/**
 * @brief WIN证书版本
 */
#define WIN_CERT_REVISION_1_0 0x0100U /**< 版本1.0 */
#define WIN_CERT_REVISION_2_0 0x0200U /**< 版本2.0 */

/**
 * @brief WIN证书结构
 */
struct win_certificate
{
    uint32_t length;    /**< 长度 */
    uint16_t revision;  /**< 版本 */
    uint16_t cert_type; /**< 类型 */
};

/** @} */

#endif /* !__ASSEMBLY__ */

/************************外部变量******************************/
/************************外部函数******************************/
/************************内联函数******************************/
/************************C++兼容性******************************/
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif /* __PE_H */
