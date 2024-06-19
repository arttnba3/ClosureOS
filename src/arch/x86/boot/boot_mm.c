/**
 * Boot stage operations related to memory management.
 * 
 * Copyright (c) 2024 arttnba3 <arttnba@gmail.com>
 * 
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
*/

#include <closureos/types.h>
#include <closureos/elf.h>
#include <boot/multiboot2.h>
#include <boot/tty.h>
#include <mm/types.h>
#include <asm/page_types.h>

static struct multiboot_tag_mmap *mmap_tag = NULL;
static uint32_t mmap_entry_nr;

static struct multiboot_mmap_entry *curr_entry = NULL;
static int curr_entry_idx = -1;
static phys_addr_t curr_avail, curr_end;

/* linear allocator without releasing, alloc a page each time */
static void* boot_mm_page_alloc_internal(void)
{
    void *res;

    /* initialization */
    if (!curr_entry) {
        for (int i = (curr_entry_idx + 1); i < mmap_entry_nr; i++) {
            if (mmap_tag->entries[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
                phys_addr_t base = mmap_tag->entries[i].addr;
                phys_addr_t end = base + mmap_tag->entries[i].len;

                if (base > end) {
                    boot_puts("[x] FATAL ERROR: "
                              "integeter overflow at parsing multiboot tags");
                    asm volatile (" hlt; ");
                }

                /**
                 * we'd like to give up the first and last partial page,
                 * as it's not enough for use to use
                */
                base = PAGE_ALIGN(base);
                end &= PAGE_MASK;
                
                /* available region may be less than 1 page, ignore */
                if ((end < base) || ((end - base) < PAGE_SIZE)) {
                    continue;
                }

                curr_avail = base;
                curr_end = end;
                curr_entry = &mmap_tag->entries[i];
                curr_entry_idx = i;
                break;
            }
        }
    }

    if (!curr_entry) {
        boot_puts("[x] FATAL ERROR: NO MEMORY AVAILABLE!");
        return NULL;
    }

    res = (void*) curr_avail;
    curr_avail += PAGE_SIZE;

    if (curr_avail == curr_end) {
        curr_entry = NULL;
    }

    return res;
}

struct multiboot_tag_elf_sections *elf_info_tag = NULL;

static bool addr_is_in_elf_range(phys_addr_t addr)
{
    return false;
}

static void* boot_mm_page_alloc(void)
{
    void *res;

    while ((res = boot_mm_page_alloc_internal())) {
        if (addr_is_in_elf_range((phys_addr_t) res)) {
            boot_printstr("\nWarning: allocated chunk: 0x");
            boot_printhex((size_t) res);
            boot_puts(" is located in kernel ELF");
            continue;
        } else {
            break;
        }
    }

    return res;
}

int boot_mm_init(multiboot_uint8_t *mbi)
{
    struct multiboot_tag *tag = (struct multiboot_tag *) (mbi + 8);
    static struct multiboot_mmap_entry *mmap_entry;

    for (tag = (struct multiboot_tag *) (mbi + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag *)
               ((multiboot_uint8_t *) tag + ((tag->size + 7) & ~7))) {
        if (tag->type == MULTIBOOT_TAG_TYPE_MMAP) {
            mmap_tag = (struct multiboot_tag_mmap *) tag;
        } else if (tag->type == MULTIBOOT_TAG_TYPE_ELF_SECTIONS) {
            elf_info_tag = (struct multiboot_tag_elf_sections*) tag;
        }
    }

    if (!mmap_tag) {
        boot_puts("[x] FAILED to find MMAP tag in multiboot info.");
        return -1;
    } else if (!elf_info_tag) {
        boot_puts("[x] FAILED to find ELF tag in multiboot info.");
        return -1;
    }

    mmap_entry_nr = (mmap_tag->size - sizeof(*mmap_tag)) / sizeof(*mmap_entry);

    for (uint32_t i = 0; i < mmap_entry_nr; i++) {
        mmap_entry = &mmap_tag->entries[i];
        if (mmap_entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            boot_printstr("[*] Memory region ");
            boot_printnum(i);
            boot_printstr(", addr: 0x");
            boot_printhex(mmap_entry->addr);
            boot_printstr(", len: 0x");
            boot_printhex(mmap_entry->len);
            boot_puts("");
        }
    }

    boot_putchar('\n');

    return -1;
}
