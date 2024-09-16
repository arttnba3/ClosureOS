/* SPDX-License-Identifier: GPL-2.0 */

#ifndef CLOSUREOS_ELF_H
#define CLOSUREOS_ELF_H

#include <closureos/types.h>

/* basic types for 32-bit ELF */

typedef u32     elf32_addr;
typedef u16     elf32_half;
typedef u32     elf32_off;
typedef s32     elf32_sword;
typedef u32     elf32_word;

/* basic types for 64-bit ELF */

typedef u64     elf64_addr;
typedef u16     elf64_half;
typedef s16     elf64_shalf;
typedef u64     elf64_off;
typedef s32     elf64_sword;
typedef u32     elf64_word;
typedef u64     elf64_xword;
typedef s64     elf64_sxword;

/* 32-bit ELF program header */
struct elf32_phdr {
    elf32_word  p_type;
    elf32_off   p_offset;
    elf32_addr  p_vaddr;
    elf32_addr  p_paddr;
    elf32_word  p_file_size;
    elf32_word  p_mem_size;
    elf32_word  p_flags;
    elf32_word  p_align;
};

/* 64-bit ELF program header */
struct elf64_phdr {
    elf64_word  p_type;
    elf64_word  p_flags;
    elf64_off   p_offset;
    elf64_addr  p_vaddr;
    elf64_addr  p_paddr;
    elf64_xword p_file_size;
    elf64_xword p_mem_size;
    elf64_xword p_align;
};

/* p_type for program header */

#define PT_NULL     0
#define PT_LOAD     1
#define PT_DYNAMIC  2
#define PT_INTEPRE  3
#define PT_NOTE     4
#define PT_SHLIB    5
#define PT_PHDR     6

/* 32-bit ELF section header */
struct elf32_shdr {
    elf32_word  sh_name;
    elf32_word  sh_type;
    elf32_word  sh_flags;
    elf32_addr  sh_addr;
    elf32_off   sh_offset;
    elf32_word  sh_size;
    elf32_word  sh_link;
    elf32_word  sh_info;
    elf32_word  sh_addr_align;
    elf32_word  sh_ent_size;
};

/* 64-bit ELF section header */
struct elf64_shdr {
    elf64_word  sh_name;
    elf64_word  sh_type;
    elf64_xword sh_flags;
    elf64_addr  sh_addr;
    elf64_off   sh_offset;
    elf64_xword sh_size;
    elf64_word  sh_link;
    elf64_word  sh_info;
    elf64_xword sh_addr_align;
    elf64_xword sh_ent_size;
};

/* sh_type for section header */

#define SHT_NULL        0
#define SHT_PROGBITS    1
#define SHT_SYMTAB      2
#define SHT_STRTAB      3
#define SHT_RELA        4
#define SHT_HASH        5
#define SHT_DYNAMIC     6
#define SHT_NOTE        7
#define SHT_NOBITS      8
#define SHT_REL         9
#define SHT_SHLIB       10
#define SHT_DYNSYM      11
#define SHT_LOPROC      0x70000000
#define SHT_HIPROC      0x7fffffff
#define SHT_LOUSER      0x80000000
#define SHT_HIUSER      0xffffffff

/* sh_flag for section header */

#define SHF_WRITE       0x1
#define SHF_ALLOC       0x2
#define SHF_EXECIINSTR  0x4
#define SHF_MASKPROC    0xf0000000

#endif // CLOSUREOS_ELF_H
