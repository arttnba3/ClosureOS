export module kernel.mm:pages;

import :layout;
import :types;
import kernel.lib;

#include <closureos/compiler.h>

export namespace mm {

class KMemCache;

#include <asm/page_types.h>

enum page_type {
    PAGE_NON_EXISTED = 0,
    PAGE_NORMAL_MEM,
    PAGE_RESERVED,
    PAGE_ACPI_RECLAIMABLE,
    PAGE_NVS,
    PAGE_BADRAM,
    PAGE_UNKNOWN,
};

/* for `type == PAGE_NORMAL_MEM` only */
enum migrate_type {
    MIGRATE_UNMOVABLE = 0,
    MIGRATE_MOVABLE,
};

/**
 * struct page
 * - representing a `physical page frame`
 * - page-aligned size
 */
struct page {
    lib::ListHead list;
    struct {
        /* for page allocator */
        unsigned type: 4;
        unsigned migrate_type: 4;
        unsigned is_free: 1; /* already in freelist */
        unsigned is_head: 1; /* head of a group of pages*/
        unsigned order: 4;
    };
    lib::atomic::atomic_t ref_count;     /* used only for PAGE_NORMAL_MEM pages */
    lib::atomic::atomic_t map_count;     /* mapped count in processes */
    lib::atomic::SpinLock lock;
    void **freelist;    /* used only when the slub is not a cpu partial */
    KMemCache *kc;  /* used only when it's a slub page */
    lib::size_t obj_nr;      /* used only when it's a slub page */

    /* unused area to make it page-aligned, maybe we can put sth else there? */
    lib::size_t unused[0];
} __attribute__((aligned(64)));

/* pages array representing all pages */
struct page *pgdb_base;
lib::size_t pgdb_page_nr;

inline constexpr lib::size_t PGDB_PG_PAGE_NR = (PAGE_SIZE / sizeof(struct page));

/* page operations */

__always_inline pfn_t page_to_pfn(struct page *p)
{
    return p - pgdb_base;
}

__always_inline struct page* pfn_to_page(pfn_t pfn)
{
    return &pgdb_base[pfn];
}

__always_inline phys_addr_t page_to_phys(struct page *p)
{
    return (p - pgdb_base) * PAGE_SIZE;
}

__always_inline virt_addr_t page_to_virt(struct page *p)
{
    return physmem_base + page_to_phys(p);
}

__always_inline phys_addr_t virt_to_phys(virt_addr_t addr)
{
    return addr - physmem_base;
}

__always_inline virt_addr_t phys_to_virt(phys_addr_t addr)
{
    return addr + physmem_base;
}

__always_inline struct page* phys_to_page(phys_addr_t addr)
{
    return &pgdb_base[(addr & PAGE_MASK) / PAGE_SIZE];
}

__always_inline struct page* virt_to_page(virt_addr_t addr)
{
    return phys_to_page(virt_to_phys(addr));
}

};
