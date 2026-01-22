/**
 * @file    efi-header.S
 * @brief   LoongArch64 EFI 启动头
 * @author  Intewell Team
 * @date    2025-01-22
 * @version 1.0
 *
 * @details 本文件实现 LoongArch64 架构的 EFI（可扩展固件接口）启动头
 *          - PE（可移植可执行）格式头
 *          - COFF（通用对象文件格式）头
 *          - 代码段和数据段定义
 *          - 支持从 EFI 固件直接启动内核
 *
 * @note 此文件由 crt1.S 包含，仅在 CONFIG_EFI_STUB 定义时生效
 * @note PE 格式用于 UEFI 固件加载和执行内核镜像
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */

/*************************** 修改历史 ****************************/
/*
 * 修改历史：
 * 2025-01-22    Intewell Team
 *               创建该文件
 */

/*************************** 头文件包含 ****************************/
#define ASM_USE
#include <pe.h>

/*************************** 宏定义 ****************************/


/*************************** 类型定义 ****************************/


/*************************** 外部声明 ****************************/


/*************************** 前向声明 ****************************/


/*************************** 模块变量 ****************************/


/*************************** 函数实现 ****************************/

/**
 * @brief EFI PE 头宏定义
 *
 * @details 定义完整的 PE32+ 格式头结构
 *          - COFF 头：机器类型、段数量、时间戳等
 *          - 可选头：入口点、段对齐、镜像大小等
 *          - 数据目录：导出表、导入表、资源表等
 *          - 段表：.text 段和 .data 段的属性和位置
 *
 * @note 此宏生成的数据结构必须符合 PE32+ 规范
 * @note PE32+ 是 64 位 Windows 和 UEFI 使用的可执行文件格式
 */
    .macro  __EFI_PE_HEADER
    .long   PE_MAGIC
.Lcoff_header:
    .short  IMAGE_FILE_MACHINE_LOONGARCH64        /* Machine: LoongArch64 */
    .short  .Lsection_count                        /* NumberOfSections */
    .long   0                                      /* TimeDateStamp */
    .long   0                                      /* PointerToSymbolTable */
    .long   0                                      /* NumberOfSymbols */
    .short  .Lsection_table - .Loptional_header    /* SizeOfOptionalHeader */
    .short  IMAGE_FILE_DEBUG_STRIPPED | \
            IMAGE_FILE_EXECUTABLE_IMAGE | \
            IMAGE_FILE_LINE_NUMS_STRIPPED          /* Characteristics */
.Loptional_header:
    .short  PE_OPT_MAGIC_PE32PLUS                  /* PE32+ format (64-bit) */
    .byte   0x02                                   /* MajorLinkerVersion */
    .byte   0x14                                   /* MinorLinkerVersion */
    .long   __inittext_end - .Lefi_header_end      /* SizeOfCode */
    .long   _kernel_vsize                          /* SizeOfInitializedData */
    .long   0                                      /* SizeOfUninitializedData */
    .long   __efistub_efi_pe_entry - _head         /* AddressOfEntryPoint */
    .long   .Lefi_header_end - _head               /* BaseOfCode */
.Lextra_header_fields:
    .quad   0                                      /* ImageBase */
    .long   PECOFF_SEGMENT_ALIGN                   /* SectionAlignment */
    .long   PECOFF_FILE_ALIGN                      /* FileAlignment */
    .short  0                                      /* MajorOperatingSystemVersion */
    .short  0                                      /* MinorOperatingSystemVersion */
    .short  LINUX_EFISTUB_MAJOR_VERSION            /* MajorImageVersion */
    .short  LINUX_EFISTUB_MINOR_VERSION            /* MinorImageVersion */
    .short  0                                      /* MajorSubsystemVersion */
    .short  0                                      /* MinorSubsystemVersion */
    .long   0                                      /* Win32VersionValue */
    .long   _end - _head                           /* SizeOfImage */
    .long   .Lefi_header_end - _head               /* SizeOfHeaders */
    .long   0                                      /* CheckSum */
    .short  IMAGE_SUBSYSTEM_EFI_APPLICATION        /* Subsystem: EFI 应用 */
    .short  0                                      /* DllCharacteristics */
    .quad   0                                      /* SizeOfStackReserve */
    .quad   0                                      /* SizeOfStackCommit */
    .quad   0                                      /* SizeOfHeapReserve */
    .quad   0                                      /* SizeOfHeapCommit */
    .long   0                                      /* LoaderFlags */
    .long   (.Lsection_table - .) / 8              /* NumberOfRvaAndSizes */
    .quad   0                                      /* ExportTable */
    .quad   0                                      /* ImportTable */
    .quad   0                                      /* ResourceTable */
    .quad   0                                      /* ExceptionTable */
    .quad   0                                      /* CertificationTable */
    .quad   0                                      /* BaseRelocationTable */

    /*
     * 段表（Section Table）
     *
     * 定义代码段和数据段的属性：
     * - .text 段：代码段，可读可执行
     * - .data 段：数据段，可读可写
     */
.Lsection_table:
    .ascii  ".text\0\0\0"
    .long   __inittext_end - .Lefi_header_end      /* VirtualSize */
    .long   .Lefi_header_end - _head               /* VirtualAddress */
    .long   __inittext_end - .Lefi_header_end      /* SizeOfRawData */
    .long   .Lefi_header_end - _head               /* PointerToRawData */
    .long   0                                      /* PointerToRelocations */
    .long   0                                      /* PointerToLineNumbers */
    .short  0                                      /* NumberOfRelocations */
    .short  0                                      /* NumberOfLineNumbers */
    .long   IMAGE_SCN_CNT_CODE | \
            IMAGE_SCN_MEM_READ | \
            IMAGE_SCN_MEM_EXECUTE                  /* Characteristics: 代码段 */

    .ascii  ".data\0\0\0"
    .long   _kernel_vsize                          /* VirtualSize */
    .long   __initdata_begin - _head               /* VirtualAddress */
    .long   _kernel_rsize                          /* SizeOfRawData */
    .long   __initdata_begin - _head               /* PointerToRawData */
    .long   0                                      /* PointerToRelocations */
    .long   0                                      /* PointerToLineNumbers */
    .short  0                                      /* NumberOfRelocations */
    .short  0                                      /* NumberOfLineNumbers */
    .long   IMAGE_SCN_CNT_INITIALIZED_DATA | \
            IMAGE_SCN_MEM_READ | \
            IMAGE_SCN_MEM_WRITE                    /* Characteristics: 数据段 */

    .set    .Lsection_count, (. - .Lsection_table) / 40
    .balign 0x10000                                /* PECOFF_SEGMENT_ALIGN */
.Lefi_header_end:
    .endm
