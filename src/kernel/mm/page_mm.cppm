export module kernel.mm:pages;

import :layout;
import :types;
import kernel.base;
import kernel.lib;

#include <closureos/compiler.h>

export namespace mm {

class KMemCache;

/**
 * Page-related definitions
 */

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
    base::size_t obj_nr;      /* used only when it's a slub page */

    /* unused area to make it page-aligned, maybe we can put sth else there? */
    base::size_t unused[0];
} __attribute__((aligned(64)));

/* pages array representing all pages */
page *pgdb_base;
base::size_t pgdb_page_nr;

inline constexpr base::size_t PGDB_PG_PAGE_NR = (PAGE_SIZE / sizeof(struct page));

/* page operations */

__always_inline pfn_t page_to_pfn(page *p)
{
    return p - pgdb_base;
}

__always_inline struct page* pfn_to_page(pfn_t pfn)
{
    return &pgdb_base[pfn];
}

__always_inline phys_addr_t page_to_phys(page *p)
{
    return (p - pgdb_base) * PAGE_SIZE;
}

__always_inline virt_addr_t page_to_virt(page *p)
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

/**
 * Page memory pool (Buddy system)
 */

__always_inline size_t buddy_page_pfn(pfn_t pfn, int order)
{
    return pfn ^ (1 << order);
}

__always_inline struct page* get_page_buddy(page *p, int order)
{
    pfn_t pfn = page_to_pfn(p);
    return pfn_to_page(buddy_page_pfn(pfn, order));
}

inline constexpr base::size_t MAX_PAGE_ORDER = 11;

class PagePool {
public:
    PagePool(void);
    ~PagePool();

    auto AllocPages(base::size_t order) -> page *;
    auto FreePages(page *page, base::size_t order) -> void;

    /* for booting stage only */
    auto Reset(void) -> void;

private:
    lib::ListHead freelist[MAX_PAGE_ORDER];
    lib::atomic::SpinLock lock;

    auto __alloc_page_direct(base::size_t order) -> page *;
    auto __alloc_pages(base::size_t order) -> page *;

    auto __free_pages(page *p, base::size_t order) -> void;

    auto __reclaim_memory(void) -> void;
};

base::uint8_t GloblPagePoolMem[sizeof(PagePool)]; /* to avoid calling global initializer, we manually point it to mem */
PagePool *GloblPagePool = (PagePool*) &GloblPagePoolMem;

PagePool::PagePool(void)
{
    /* do nothing */
}

PagePool::~PagePool(void)
{
    /* do nothing */
}

auto PagePool::__alloc_page_direct(base::size_t order) -> page *
{
    page *p = nullptr;
    base::size_t allocated = order;

    while (allocated < MAX_PAGE_ORDER) {
        if (!lib::list_empty(&this->freelist[allocated])) {
            p = lib::list_entry(this->freelist[allocated].next, &page::list);
            lib::list_del(&p->list);
            break;
        } else {
            allocated++;
        }
    }

    /* failed */
    if (!p) {
        goto out;
    }

    /* it means that we acquire pages from higher order */
    if (allocated != order) {
        /* put half pages back to buddy */
        allocated--;
        while (allocated >= order) {
            lib::list_add_next(&this->freelist[allocated],
                               &get_page_buddy(p, allocated)->list);
            allocated--;
        }
    }

out:
    return p;
}

auto PagePool::__alloc_pages(base::size_t order) -> page *
{
    page *p = nullptr;
    bool redo = false;

    if (order >= MAX_PAGE_ORDER) {
        return nullptr;
    }

    this->lock.Lock();

redo:
    /* try to alloc directly */
    p = this->__alloc_page_direct(order);
    if (p) {
        for (int i = 0; i < (1 << order); i++) {
            p[i].is_head = false;
            p[i].order = order;
            p->is_free = false;
            list_head_init(&p[i].list);
        }

        p->is_head = true;

        goto out;
    }

    /* failed to allocate! try to reclaim memory... */
    if (!redo) {
        this->__reclaim_memory();
        redo = true;
        goto redo;
    }

out:
    this->lock.UnLock();

    return p;
}

auto PagePool::__free_pages(page *p, base::size_t order) -> void
{
    if (!p) {
        return;
    }

    if (order >= MAX_PAGE_ORDER) {
        return;
    }

    this->lock.Lock();

    /* try to combine nearby pages */
    while (order < (MAX_PAGE_ORDER - 1)) {
        page *buddy;

        buddy = get_page_buddy(p, order);
        if (buddy->type == PAGE_NORMAL_MEM && buddy->is_free && buddy->is_head){
            list_del(&buddy->list);
            if (buddy < p) {
                p->is_head = false;
                p = buddy;
            }
            order++;
            continue;
        }

        /* we can't combine forward or backward, just break */
        break;
    }

    for (int i = 0; i < (1 << order); i++) {
        p[i].is_head = false;
        p[i].is_free = true;
        p[i].order = order;
        p->ref_count = p->map_count = -1;
    }
    p->is_head = true;

    list_add_next(&(this->freelist[order]), &p->list);

    this->lock.UnLock();
}

auto PagePool::__reclaim_memory(void) -> void
{
    /* TODO: reclame movable memory pages */
}

auto PagePool::AllocPages(base::size_t order) -> page *
{
    return this->__alloc_pages(order);
}

auto PagePool::FreePages(page *page, base::size_t order) -> void
{
    this->__free_pages(page, order);
}

auto PagePool::Reset(void) -> void
{
    for (auto i = 0; i < MAX_PAGE_ORDER; i++) {
        lib::list_head_init(&this->freelist[i]);
    }

    this->lock.Reset();
}

};
