/**
 * Boot stage operations related to memory management.
 * 
 * Copyright (c) 2024 arttnba3 <arttnba@gmail.com>
 * 
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
*/

#include <closureos/types.h>
#include <closureos/elf.h>
#include <closureos/err.h>
#include <boot/multiboot2.h>
#include <boot/tty.h>
#include <boot/string.h>
#include <closureos/mm/mm_types.h>
#include <closureos/mm/layout.h>
#include <closureos/mm/pages.h>
#include <asm/page_types.h>

static struct multiboot_tag_mmap *mmap_tag = NULL;
static uint32_t mmap_entry_nr;

struct multiboot_mmap_entry *curr_entry = NULL;
int curr_entry_idx = -1;
phys_addr_t curr_avail, curr_end;

/**
 * Linear allocator without releasing, alloc a page each time.
 * Note that the result could be physical 0, NULL should not be use as failure,
 * but a -EMOMEM or some other error number should be returned.
*/
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
        return ERR_PTR(-ENOMEM);
    }

    res = (void*) curr_avail;
    curr_avail += PAGE_SIZE;

    if (curr_avail == curr_end) {
        curr_entry = NULL;
    }

    return res;
}

struct multiboot_tag_elf_sections *elf_info_tag = NULL;
phys_addr_t multiboot_tag_start, multiboot_tag_end;

static bool addr_is_in_used_range(phys_addr_t addr)
{
    struct elf64_shdr *shdr;
    void *shdr_end;
    phys_addr_t seg_start, seg_end;

    /* in ELF range */
    shdr_end = (void*) ((phys_addr_t) &elf_info_tag->sections
               + elf_info_tag->num * sizeof(*shdr));

    for (shdr = (void*) &elf_info_tag->sections;
         (void*) shdr < shdr_end;
         shdr = (void*) ((phys_addr_t) shdr + sizeof(*shdr))) {

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
        if ((addr >= (phys_addr_t) boot_fb_base)
            && (addr <= (phys_addr_t) boot_fb_end)) {
            return true;
        }
    }

    /* in multiboot tags */
    if (addr >= multiboot_tag_start && addr <= multiboot_tag_end) {
        return true;
    }

    return false;
}

static void* boot_mm_page_alloc(void)
{
    void *res;

    do {
        res = boot_mm_page_alloc_internal();
    } while (addr_is_in_used_range((phys_addr_t) res));

    return res;
}

/**
 * map a single page on page table,
 * note that we SHOUDN'T FORGET to set the attr for each level's entry
*/
int boot_mm_pgtable_map(phys_addr_t pgtable,
                        virt_addr_t va,
                        phys_addr_t pa, 
                        page_attr_t attr)
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
            pgd[pgd_i] = (pgd_t) NULL;
            return -ENOMEM;
        }

        boot_memset((void*) pgd[pgd_i], 0, PAGE_SIZE);
        pgd[pgd_i] |= attr;
    }

    pud = (pud_t*) (pgd[pgd_i] & PAGE_MASK);
    if (!pud[pud_i]) {
        pud[pud_i] = (pud_t) boot_mm_page_alloc();
        if (IS_ERR_PTR((void*) pud[pud_i])) {
            pud[pud_i] = (pud_t) NULL;
            return -ENOMEM;
        }

        boot_memset((void*) pud[pud_i], 0, PAGE_SIZE);
        pud[pud_i] |= attr;
    }

    pmd = (pmd_t*) (pud[pud_i] & PAGE_MASK);
    if (!pmd[pmd_i]) {
        pmd[pmd_i] = (pmd_t) boot_mm_page_alloc();
        if (IS_ERR_PTR((void*) pmd[pmd_i])) {
            pmd[pmd_i] = (pmd_t) NULL;
            return -ENOMEM;
        }

        boot_memset((void*) pmd[pmd_i], 0, PAGE_SIZE);
        pmd[pmd_i]|= attr;
    }

    pte = (pte_t*) (pmd[pmd_i] & PAGE_MASK);
    pte[pte_i] = pa | attr;

    return 0;
}

phys_addr_t boot_kern_pgtable;

static void boot_mm_load_pgtable(phys_addr_t pgtable)
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
static int boot_mm_pgtable_init(void)
{
    struct elf64_shdr *shdr;
    void *shdr_end;
    phys_addr_t seg_phys_start, seg_phys_end;
    virt_addr_t seg_virt_start, seg_virt_end;
    page_attr_t pte_attr;
    phys_addr_t physmem_start, physmem_end;
    int ret;

    boot_kern_pgtable = (phys_addr_t) boot_mm_page_alloc();
    if (IS_ERR_PTR((void*) boot_kern_pgtable)) {
        boot_puts("[x] FAILED to allocate new page table!");
        return -ENOMEM;
    }
    boot_memset((void*) boot_kern_pgtable, 0, PAGE_SIZE);

    /* map kernel ELF */
    shdr_end = (void*) ((phys_addr_t) &elf_info_tag->sections
               + elf_info_tag->num * sizeof(*shdr));

    for (shdr = (void*) &elf_info_tag->sections;
         (void*) shdr < shdr_end;
         shdr = (void*) ((phys_addr_t) shdr + sizeof(*shdr))) {

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

                seg_phys_start = (phys_addr_t) boot_mm_page_alloc();
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
        seg_phys_start = ((phys_addr_t) boot_fb_base) & PAGE_MASK;
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
        phys_addr_t base = mmap_tag->entries[i].addr;
        phys_addr_t end = base + mmap_tag->entries[i].len;
        virt_addr_t vaddr = base + KERN_DIRECT_MAP_REGION_BASE;

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
            if (vaddr > KERN_DIRECT_MAP_REGION_END) {   /* out of 128TB */
                continue;
            }

            boot_mm_pgtable_map(boot_kern_pgtable,
                                vaddr,
                                base,
                                PTE_ATTR_P | PTE_ATTR_RW);

            base += PAGE_SIZE;
            vaddr += PAGE_SIZE;
        }
    }

    /* load the new page table now! */
    boot_mm_load_pgtable(boot_kern_pgtable);

    /* map for `struct page` array */
    physmem_end = PAGE_ALIGN(physmem_end);
    pgdb_base = (void*) KERN_PAGE_DATABASE_REGION_BASE;
    pgdb_page_nr = (physmem_end - physmem_start) / PAGE_SIZE;

    for (size_t i = 0; physmem_start < physmem_end; i += PGDB_PG_PAGE_NR) {
        void *new_page = boot_mm_page_alloc();

        if(IS_ERR_PTR(new_page)) {
            return PTR_ERR(new_page);
        }

        boot_memset(new_page, 0, PAGE_SIZE);
        ret = boot_mm_pgtable_map(boot_kern_pgtable,
                                  (virt_addr_t) &pgdb_base[i],
                                  physmem_start,
                                  PTE_ATTR_P | PTE_ATTR_RW);
        if (ret < 0) {
            return ret;
        }

        physmem_start += PGDB_PG_PAGE_NR * PAGE_SIZE;
    }

    /**
     * as we set all page to 0 at the begining, we don't need to care about hole
     * because the type for memory hole is 0 in our design
    */
    for (int i = 0; i < mmap_entry_nr; i++) {
        phys_addr_t base = mmap_tag->entries[i].addr;
        phys_addr_t end = base + mmap_tag->entries[i].len;
        size_t pfn;

        while (base < end) {
            pfn = base / PAGE_SIZE;

            switch (mmap_tag->entries[i].type) {
            case MULTIBOOT_MEMORY_AVAILABLE:
                pgdb_base[pfn].type = PAGE_NORMAL_MEM;
                break;
            case MULTIBOOT_MEMORY_RESERVED:
                pgdb_base[pfn].type = PAGE_RESERVED;
                break;
            case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
                pgdb_base[pfn].type = PAGE_ACPI_RECLAIMABLE;
                break;
            case MULTIBOOT_MEMORY_NVS:
                pgdb_base[pfn].type = PAGE_NVS;
                break;
            case MULTIBOOT_MEMORY_BADRAM:
                pgdb_base[pfn].type = PAGE_BADRAM;
                break;
            default:
                boot_printstr("[!] Warning: unknown memory type [");
                boot_printnum(mmap_tag->entries[i].type);
                boot_printstr("] at addr: 0x");
                boot_printhex(base);
                boot_putchar('\n');
                break;
            }

            base += PAGE_SIZE;
        }
    }

    /* TODO: calculate ref-count for used pages now */

    return 0;
}

int boot_mm_init(multiboot_uint8_t *mbi)
{
    struct multiboot_tag *tag = (struct multiboot_tag *) (mbi + 8);
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
    multiboot_tag_start = (phys_addr_t) mbi & PAGE_MASK;

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

    return 0;
}
