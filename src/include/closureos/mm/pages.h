#ifndef MM_PAGES_H
#define MM_PAGES_H

#include <closureos/types.h>
#include <closureos/list.h>
#include <closureos/lock.h>
#include <closureos/mm/mm_types.h>
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
    struct list_head list;
    struct {
        /* for page allocator */
        unsigned type: 4;
        unsigned migrate_type: 4;
        unsigned is_free: 1; /* already in freelist */
        unsigned is_head: 1; /* head of a group of pages*/
        unsigned order: 4;
    };
    atomic_t ref_count;     /* used only for PAGE_NORMAL_MEM pages */
    atomic_t map_count;     /* mapped count in processes */
    spinlock_t lock;
    void **freelist;    /* used only when the slub is not a cpu partial */
    struct kmem_cache *kc;  /* used only when it's a slub page */
    size_t obj_nr;      /* used only when it's a slub page */

    /* unused area to make it page-aligned, maybe we can put sth else there? */
    size_t unused[0];
} __attribute__((aligned(64)));

/* pages array representing all pages */
extern struct page *pgdb_base;
extern size_t pgdb_page_nr;
extern spinlock_t pgdb_lock;

#define MAX_PAGE_ORDER 11
extern struct list_head freelist[MAX_PAGE_ORDER];

static __always_inline struct page *get_head_page(struct page *p)
{
    return p->is_head 
          ? p 
          : (struct page*)((virt_addr_t) p & ~((sizeof(*p)*(1 << p->order))-1));
}

/* number of `struct page` in a page frame */
#define PGDB_PG_PAGE_NR (PAGE_SIZE / sizeof(struct page))

extern virt_addr_t physmem_base, vmremap_base, kernel_base;

static __always_inline pfn_t page_to_pfn(struct page *p)
{
    return p - pgdb_base;
}

static __always_inline struct page* pfn_to_page(pfn_t pfn)
{
    return &pgdb_base[pfn];
}

static __always_inline phys_addr_t page_to_phys(struct page *p)
{
    return (p - pgdb_base) * PAGE_SIZE;
}

static __always_inline virt_addr_t page_to_virt(struct page *p)
{
    return physmem_base + page_to_phys(p);
}

static __always_inline phys_addr_t virt_to_phys(virt_addr_t addr)
{
    return addr - physmem_base;
}

static __always_inline virt_addr_t phys_to_virt(phys_addr_t addr)
{
    return addr + physmem_base;
}

static __always_inline struct page* phys_to_page(phys_addr_t addr)
{
    return &pgdb_base[(addr & PAGE_MASK) / PAGE_SIZE];
}

static __always_inline struct page* virt_to_page(virt_addr_t addr)
{
    return phys_to_page(virt_to_phys(addr));
}

extern struct page *alloc_pages(int order);
extern void free_pages(struct page *p, int order);

static __always_inline void get_page(struct page *p)
{
    atomic_inc(&get_head_page(p)->ref_count);
}

static __always_inline void put_page(struct page *p)
{
    p = get_head_page(p);

    if (atomic_dec(&p->ref_count) < 0) {
        free_pages(p, p->order);
    }
}

#endif // MM_PAGES_H
