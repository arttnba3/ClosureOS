/**
 * Boot stage operations related to memory management.
 * 
 * Copyright (c) 2024 arttnba3 <arttnba3@outlook.com>
 * 
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
*/

import kernel.base;
import kernel.mm;
import kernel.lib;

extern "C" {

#include <closureos/elf.h>
#include <closureos/err.h>
#include <boot/multiboot2.h>
#include <boot/tty.h>
#include <boot/string.h>
#include <asm/page_types.h>

}

static struct multiboot_tag_mmap *mmap_tag = nullptr;
static uint32_t mmap_entry_nr;

struct multiboot_mmap_entry *curr_entry = nullptr;
int curr_entry_idx = -1;
mm::phys_addr_t curr_avail, curr_end;

/**
 * Linear allocator without releasing, alloc a page each time.
 * Note that the result could be physical 0, NULL should not be use as failure,
 * but a -EMOMEM or some other error number should be returned.
*/
static auto boot_mm_page_alloc_internal(void) -> void*
{
    void *res;

    /* initialization */
    if (!curr_entry) {
        for (int i = (curr_entry_idx + 1); i < mmap_entry_nr; i++) {
            if (mmap_tag->entries[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
                mm::phys_addr_t base = mmap_tag->entries[i].addr;
                mm::phys_addr_t end = base + mmap_tag->entries[i].len;

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
        return ERR_PTR(-ENOMEM);
    }

    res = (void*) curr_avail;
    curr_avail += PAGE_SIZE;

    if (curr_avail == curr_end) {
        curr_entry = nullptr;
    }

    return res;
}

static struct multiboot_tag_elf_sections *elf_info_tag = nullptr;
mm::phys_addr_t multiboot_tag_start, multiboot_tag_end;

static auto addr_is_in_used_range(mm::phys_addr_t addr) -> bool
{
    struct elf64_shdr *shdr;
    void *shdr_end;
    mm::phys_addr_t seg_start, seg_end;

    /* in ELF range */
    shdr_end = (void*) ((mm::phys_addr_t) &elf_info_tag->sections
               + elf_info_tag->num * sizeof(*shdr));

    for (shdr = (elf64_shdr*) &elf_info_tag->sections;
         (void*) shdr < shdr_end;
         shdr = (elf64_shdr*) ((mm::phys_addr_t) shdr + sizeof(*shdr))) {

        if ((shdr->sh_type != SHT_PROGBITS)
            || !(shdr->sh_flags & SHF_ALLOC)) {
            continue;
        }

        seg_start = 0x100000 + shdr->sh_offset;
        seg_end = PAGE_ALIGN(seg_start + shdr->sh_size);

        if (addr >= seg_start && addr < seg_end) {
            return true;
        }
    }

    /* in frame buffer */
    if (boot_tty_has_fb()) {
        if ((addr >= (mm::phys_addr_t) boot_fb_base)
            && (addr <= (mm::phys_addr_t) boot_fb_end)) {
            return true;
        }
    }

    /* in multiboot tags */
    if ((addr >= multiboot_tag_start) && (addr <= multiboot_tag_end)) {
        return true;
    }

    return false;
}

static auto boot_mm_page_alloc(void) -> void*
{
    void *res;

    do {
        res = boot_mm_page_alloc_internal();
    } while (addr_is_in_used_range((mm::phys_addr_t) res));

    return res;
}

/**
 * map a single page on page table,
 * note that we SHOUDN'T FORGET to set the attr for each level's entry
*/
static auto boot_mm_pgtable_map(mm::phys_addr_t pgtable,
                                mm::virt_addr_t va,
                                mm::phys_addr_t pa, 
                                mm::page_attr_t attr) -> int
{
    pgd_t *pgd;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    int pgd_i = PGD_ENTRY(va);
    int pud_i = PUD_ENTRY(va);
    int pmd_i = PMD_ENTRY(va);
    int pte_i = PTE_ENTRY(va);

    pgd = (pgd_t*) pgtable;
    if (!pgd[pgd_i]) {
        pgd[pgd_i] = (pgd_t) boot_mm_page_alloc();
        if (IS_ERR_PTR((void*) pgd[pgd_i])) {
            pgd[pgd_i] = (pgd_t) nullptr;
            return -ENOMEM;
        }

        boot_memset((void*) ((mm::phys_addr_t) pgd[pgd_i]), 0 ,PAGE_SIZE);
        pgd[pgd_i] |= PDE_DEFAULT;
    }

    pud = (pud_t*) (pgd[pgd_i] & PAGE_MASK);
    if (!pud[pud_i]) {
        pud[pud_i] = (pud_t) boot_mm_page_alloc();
        if (IS_ERR_PTR((void*) pud[pud_i])) {
            pud[pud_i] = (pud_t) nullptr;
            return -ENOMEM;
        }

        boot_memset((void*) ((mm::phys_addr_t) pud[pud_i]), 0 ,PAGE_SIZE);
        pud[pud_i] |= PDE_DEFAULT;
    }

    pmd = (pmd_t*) (pud[pud_i] & PAGE_MASK);
    if (!pmd[pmd_i]) {
        pmd[pmd_i] = (pmd_t) boot_mm_page_alloc();
        if (IS_ERR_PTR((void*) pmd[pmd_i])) {
            pmd[pmd_i] = (pmd_t) nullptr;
            return -ENOMEM;
        }

        boot_memset((void*) ((mm::phys_addr_t) pmd[pmd_i]), 0 ,PAGE_SIZE);
        pmd[pmd_i] |= PDE_DEFAULT;
    }

    pte = (pte_t*) (pmd[pmd_i] & PAGE_MASK);
    pte[pte_i] = pa | attr;

    return 0;
}

mm::phys_addr_t boot_kern_pgtable;

static auto boot_mm_load_pgtable(mm::phys_addr_t pgtable) -> void
{
    asm volatile(
        "mov    %0, %%rax;"
        "mov    %%rax, %%cr3;"
        :
        : "a" (pgtable)
    );
}

/**
 * Map physical memory region to corresponding virtual memory region
*/
static auto boot_mm_pgtable_init(void) -> int
{
    struct elf64_shdr *shdr;
    void *shdr_end;
    mm::phys_addr_t seg_phys_start, seg_phys_end;
    mm::virt_addr_t seg_virt_start, seg_virt_end;
    mm::page_attr_t pte_attr;
    mm::phys_addr_t physmem_start, physmem_end;
    mm::Page *pgdb_base;
    base::size_t pgdb_page_nr;
    int ret;

    boot_kern_pgtable = (mm::phys_addr_t) boot_mm_page_alloc();
    if (IS_ERR_PTR((void*) boot_kern_pgtable)) {
        boot_puts("[x] FAILED to allocate new page table!");
        return -ENOMEM;
    }
    boot_memset((void*) boot_kern_pgtable, 0, PAGE_SIZE);

    /* map kernel ELF */
    shdr_end = (void*) ((mm::phys_addr_t) &elf_info_tag->sections
               + elf_info_tag->num * sizeof(*shdr));

    for (shdr = (elf64_shdr*) &elf_info_tag->sections;
         (void*) shdr < shdr_end;
         shdr = (elf64_shdr*) ((mm::phys_addr_t) shdr + sizeof(*shdr))) {

        if (!(shdr->sh_flags & SHF_ALLOC)) {
            continue;
        }

        if (shdr->sh_type & SHT_PROGBITS) {    /* for .text, .data, .rodata */
            seg_phys_start = 0x100000 + shdr->sh_offset - PAGE_SIZE;
            seg_phys_end = PAGE_ALIGN(seg_phys_start + shdr->sh_size);
            seg_virt_start =shdr->sh_addr;

            while (seg_phys_start < seg_phys_end) {
                pte_attr = PTE_ATTR_P;
                if (shdr->sh_flags & SHF_WRITE) {
                    pte_attr |= PTE_ATTR_RW;
                }

                ret = boot_mm_pgtable_map(boot_kern_pgtable,
                                          seg_virt_start,
                                          seg_phys_start,
                                          pte_attr);
                if (ret < 0) {  /* out of memory */
                    return ret;
                }

                seg_phys_start += PAGE_SIZE;
                seg_virt_start += PAGE_SIZE;
            }
        } else if (shdr->sh_type & SHT_NOBITS) {   /* for .bss */
            seg_virt_start = shdr->sh_addr;
            seg_virt_end = PAGE_ALIGN(seg_virt_start + shdr->sh_size);

            while (seg_virt_start < seg_virt_end) {
                pte_attr = PTE_ATTR_P;
                if (shdr->sh_flags & SHF_WRITE) {
                    pte_attr |= PTE_ATTR_RW;
                }

                seg_phys_start = (mm::phys_addr_t) boot_mm_page_alloc();
                if (IS_ERR_PTR((void*) seg_phys_start)) {
                    return -ENOMEM;
                }

                boot_memset((void*) seg_phys_start, 0, PAGE_SIZE);
                ret = boot_mm_pgtable_map(boot_kern_pgtable,
                                          seg_virt_start,
                                          seg_phys_start,
                                          pte_attr);
                if (ret < 0) {  /* out of memory */
                    return ret;
                }

                seg_virt_start += PAGE_SIZE;
            }
        } else {    /* unknown but we need to allocate??? */
            boot_puts("[x] Unknown segment. ELF mapping stopped.");
            return -EFAULT;
        }
    }

    /* map for frame buffer */
    if (boot_tty_has_fb()) {
        seg_phys_start = ((mm::phys_addr_t) boot_fb_base) & PAGE_MASK;
        seg_virt_start = seg_phys_start;
        seg_phys_end = PAGE_ALIGN(seg_phys_start + boot_tty_fb_sz());
        while (seg_phys_start < seg_phys_end) {
            ret = boot_mm_pgtable_map(boot_kern_pgtable,
                                      seg_virt_start,
                                      seg_phys_start,
                                      PTE_ATTR_P | PTE_ATTR_RW);
            if (ret < 0) {  /* out of memory */
                return ret;
            }

            seg_phys_start += PAGE_SIZE;
            seg_virt_start += PAGE_SIZE;
        }
    }

    /* map for direct mapping area */
    physmem_start = physmem_end = 0x0000000000000000;

    for (int i = 0; i < mmap_entry_nr; i++) {
        mm::phys_addr_t base = mmap_tag->entries[i].addr & PAGE_MASK;
        mm::phys_addr_t end = base + mmap_tag->entries[i].len;
        mm::virt_addr_t vaddr = base + mm::KERN_DIRECT_MAP_REGION_BASE;

        if (end > physmem_end) {
            physmem_end = end;
        }

        if (vaddr < base) {
            boot_printstr("[x] FATAL: memory region base 0x");
            boot_printhex(base);
            boot_puts(" has caused an integer overflow in memory mapping.");
            asm volatile(" hlt ");
        }

        while (base < end) {
            if (vaddr > mm::KERN_DIRECT_MAP_REGION_END) {   /* out of 64TB */
                break;
            }

            ret = boot_mm_pgtable_map(boot_kern_pgtable,
                                vaddr,
                                base,
                                PTE_ATTR_P | PTE_ATTR_RW);
            if (ret < 0) {
                return ret;
            }

            base += PAGE_SIZE;
            vaddr += PAGE_SIZE;
        }
    }

    /* map for page database (`struct page` array) */
    pgdb_base = (mm::Page*) mm::KERN_PAGE_DATABASE_REGION_BASE;
    pgdb_page_nr = (physmem_end - physmem_start) / PAGE_SIZE;

    for (base::size_t i = 0; i < pgdb_page_nr; i += mm::PGDB_PG_PAGE_NR) {
        void *new_page = boot_mm_page_alloc();

        if(IS_ERR_PTR(new_page)) {
            return PTR_ERR(new_page);
        }

        boot_memset((void*) new_page, 0, PAGE_SIZE);
        ret = boot_mm_pgtable_map(boot_kern_pgtable,
                                  (mm::virt_addr_t) &pgdb_base[i],
                                  (mm::phys_addr_t) new_page,
                                  PTE_ATTR_P | PTE_ATTR_RW);
        if (ret < 0) {
            return ret;
        }
    }

    /* load the new page table now! */
    boot_mm_load_pgtable(boot_kern_pgtable);

    /* init for some vals */
    mm::physmem_base = mm::KERN_DIRECT_MAP_REGION_BASE;
    mm::kernel_base = mm::KERN_SEG_TEXT_REGION_START;
    mm::vmremap_base = mm::KERN_DYNAMIC_MAP_REGION_BASE;
    mm::pgdb_base = pgdb_base;
    mm::pgdb_page_nr = pgdb_page_nr;

    /* remap some variables we may still use */
    elf_info_tag = (multiboot_tag_elf_sections*) mm::phys_to_virt((mm::phys_addr_t) elf_info_tag);
    mmap_tag = (multiboot_tag_mmap*) mm::phys_to_virt((mm::phys_addr_t) mmap_tag);

    return 0;
}

static auto boot_mm_page_database_init(void) -> int
{
    int ret;

    /**
     * as we set all page to 0 at the beginning,we don't need to care about hole
     * because the type for memory hole is 0 in our design
    */
    for (int i = 0; i < mmap_entry_nr; i++) {
        mm::phys_addr_t base = mmap_tag->entries[i].addr;
        mm::phys_addr_t end = base + mmap_tag->entries[i].len;
        base::size_t pfn;

        while (base < end) {
            pfn = base / PAGE_SIZE;
            /* initialized value for every page */
            mm::pgdb_base[pfn].migrate_type = mm::MIGRATE_UNMOVABLE;    /* temporarily only this only */
            mm::pgdb_base[pfn].lock.Reset();
            mm::pgdb_base[pfn].kc = nullptr;
            mm::pgdb_base[pfn].pool = nullptr;
            mm::pgdb_base[pfn].freelist = nullptr;
            mm::pgdb_base[pfn].obj_nr = 0;
            lib::list_head_init(&mm::pgdb_base[pfn].list);

            switch (mmap_tag->entries[i].type) {
            case MULTIBOOT_MEMORY_AVAILABLE:
                mm::pgdb_base[pfn].type = mm::PAGE_NORMAL_MEM;

                if (base < curr_avail || addr_is_in_used_range(base)) {
                    /* used page */
                    lib::atomic::atomic_set(&mm::pgdb_base[pfn].ref_count, 0);
                } else {
                    /* free page */
                    lib::atomic::atomic_set(&mm::pgdb_base[pfn].ref_count, -1);
                }
                break;
            case MULTIBOOT_MEMORY_RESERVED:
                mm::pgdb_base[pfn].type = mm::PAGE_RESERVED;
                lib::atomic::atomic_set(&mm::pgdb_base[pfn].ref_count, 0);
                break;
            case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
                mm::pgdb_base[pfn].type = mm::PAGE_ACPI_RECLAIMABLE;
                lib::atomic::atomic_set(&mm::pgdb_base[pfn].ref_count, 0);
                break;
            case MULTIBOOT_MEMORY_NVS:
                mm::pgdb_base[pfn].type = mm::PAGE_NVS;
                lib::atomic::atomic_set(&mm::pgdb_base[pfn].ref_count, 0);
                break;
            case MULTIBOOT_MEMORY_BADRAM:
                mm::pgdb_base[pfn].type = mm::PAGE_BADRAM;
                lib::atomic::atomic_set(&mm::pgdb_base[pfn].ref_count, 0);
                break;
            default:
                mm::pgdb_base[pfn].type = mm::PAGE_UNKNOWN;
                lib::atomic::atomic_set(&mm::pgdb_base[pfn].ref_count, 0);
                /*
                boot_printstr("[!] Warning: unknown memory type [");
                boot_printnum(mmap_tag->entries[i].type);
                boot_printstr("] at addr: 0x");
                boot_printhex(base);
                boot_putchar('\n');
                */
                break;
            }

            base += PAGE_SIZE;
        }
    }

    return 0;
}

auto boot_mm_init(multiboot_uint8_t *mbi) -> int
{
    struct multiboot_tag *tag;
    static struct multiboot_mmap_entry *mmap_entry;
    int ret;

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

    /* calculate for multiboot tags range */
    multiboot_tag_start = (mm::phys_addr_t) mbi & PAGE_MASK;

    for (tag = (struct multiboot_tag *) (mbi + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag *)
               ((multiboot_uint8_t *) tag + ((tag->size + 7) & ~7))) {
        /* just do nothing*/
    }

    multiboot_tag_end = PAGE_ALIGN((page_attr_t) tag);

    if ((ret = boot_mm_pgtable_init()) < 0) {
        boot_printstr("[x] FAILED to initialize page table, errno: ");
        boot_printnum(ret);
        boot_putchar('\n');
    }

    if ((ret = boot_mm_page_database_init()) < 0) {
        boot_printstr("[x] FAILED to initialize page database, errno: ");
        boot_printnum(ret);
        boot_putchar('\n');
    }

    auto val = mm::KERN_DIRECT_MAP_REGION_BASE;

    return val;
}
