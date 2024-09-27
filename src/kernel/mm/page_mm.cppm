export module kernel.mm:pages;

import :layout;
import :types;
import kernel.base;
import kernel.lib;

#include <closureos/compiler.h>

export namespace mm {

class KMemCache;
class PagePool;

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
 * struct Page
 * - representing a `physical page frame`
 * - page-aligned size
 * 
 * NOTE: we ONLY store some critical information at the head page, as it takes too long to assign the value
 * to every page, but only recording it at the head and to find the head at use is enough.
 */
struct Page {
public:
    lib::ListHead list;
    struct {
        /* for page allocator */
        unsigned type: 4;
        unsigned migrate_type: 4;
        unsigned is_free: 1; /* already in freelist */
        unsigned is_head: 1; /* head of a group of pages*/
        unsigned order: 4;
    };
    lib::atomic::atomic_t ref_count;     /* -1 for free */
    lib::atomic::atomic_t map_count;     /* mapped count in processes */
    lib::atomic::SpinLock lock;
    void **freelist;    /* used only when the slub is not a cpu partial */
    KMemCache *kc;  /* used only when it's a slub page */
    PagePool *pool; /* SHOULD remains unchanged after initialization */
    base::size_t obj_nr;      /* used only when it's a slub page */

    /* unused area to make it page-aligned, maybe we can put sth else there? */
    base::size_t unused[0];
} __attribute__((aligned(64)));

/* pages array representing all pages */
Page *pgdb_base;
base::size_t pgdb_page_nr;

inline constexpr base::size_t PGDB_PG_PAGE_NR = (PAGE_SIZE / sizeof(Page));

/* page operations */

__always_inline pfn_t page_to_pfn(Page *p)
{
    return p - pgdb_base;
}

__always_inline Page* pfn_to_page(pfn_t pfn)
{
    return &pgdb_base[pfn];
}

__always_inline phys_addr_t page_to_phys(Page *p)
{
    return (p - pgdb_base) * PAGE_SIZE;
}

__always_inline virt_addr_t page_to_virt(Page *p)
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

__always_inline Page* phys_to_page(phys_addr_t addr)
{
    return &pgdb_base[(addr & PAGE_MASK) / PAGE_SIZE];
}

__always_inline Page* virt_to_page(virt_addr_t addr)
{
    return phys_to_page(virt_to_phys(addr));
}

__always_inline struct Page *get_head_page(struct Page *p)
{
    return p->is_head 
          ? p 
          : (struct Page*)((virt_addr_t) p & ~((sizeof(*p)*(1 << p->order))-1));
}

/**
 * Page memory pool (Buddy system)
 */

__always_inline size_t buddy_page_pfn(pfn_t pfn, int order)
{
    return pfn ^ (1 << order);
}

__always_inline Page* get_page_buddy(Page *p, int order)
{
    pfn_t pfn = page_to_pfn(p);
    return pfn_to_page(buddy_page_pfn(pfn, order));
}

inline constexpr base::size_t MAX_PAGE_ORDER = 11;

class PagePool {
public:
    PagePool(void);
    ~PagePool();

    auto AllocPages(base::size_t order) -> Page *;
    auto FreePages(Page *page, base::size_t order) -> void;

    /* for booting stage only */

    auto Init(void) -> void;
    auto AddPages(Page *page, base::size_t order) -> void;

private:
    lib::ListHead freelist[MAX_PAGE_ORDER];
    lib::atomic::SpinLock lock;

    auto __reinit_page(Page *p, base::size_t order, bool free) -> void;

    auto __alloc_page_direct(base::size_t order) -> Page *;
    auto __alloc_pages(base::size_t order) -> Page *;

    auto __free_pages(Page *p, base::size_t order) -> void;

    auto __reclaim_memory(void) -> void;
};

enum page_pool_types {
    PAGE_POOL_TYPE_NORMAL = 0,
    PAGE_POOL_TYPE_NR,
};

base::uint8_t GloblPagePoolMem[sizeof(PagePool)]; /* to avoid calling global initializer, we manually point it to mem */
PagePool *GloblPagePool = (PagePool*) &GloblPagePoolMem;
PagePool *GloblPagePoolGroup[PAGE_POOL_TYPE_NR] = {
    (PagePool*) &GloblPagePoolMem,
};

__always_inline void get_page(struct Page *p)
{
    lib::atomic::atomic_inc(&get_head_page(p)->ref_count);
}

__always_inline void put_page(struct Page *p)
{
    p = get_head_page(p);

    if (lib::atomic::atomic_dec(&p->ref_count) < 0) {
        p->pool->FreePages(p, p->order);
    }
}

PagePool::PagePool(void)
{
    /* do nothing */
}

PagePool::~PagePool(void)
{
    /* do nothing */
}

auto PagePool::__reinit_page(Page *p, base::size_t order, bool free) -> void
{
    for (auto i = 0; i < (1 << order); i++) {
        p[i].is_free = free;
        p[i].order = order;
        p[i].freelist = nullptr;
        p[i].kc = nullptr;
        p[i].is_head = false;
        lib::atomic::atomic_set(&p->ref_count, -1);
        lib::atomic::atomic_set(&p->map_count, -1);
    }

    p[0].is_head = true;
}

auto PagePool::__alloc_page_direct(base::size_t order) -> Page *
{
    Page *p = nullptr;
    Page *buddy;
    base::size_t allocated = order;

    while (allocated < MAX_PAGE_ORDER) {
        if (!lib::list_empty(&this->freelist[allocated])) {
            p = lib::list_entry(this->freelist[allocated].next, &Page::list);
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
        do {
            allocated--;
            buddy = get_page_buddy(p, allocated);
            this->__reinit_page(buddy, allocated, true);
            lib::list_add_next(&this->freelist[allocated], &buddy->list);
        } while (allocated > order);
    }

    this->__reinit_page(p, allocated, false);

out:
    return p;
}

auto PagePool::__alloc_pages(base::size_t order) -> Page *
{
    Page *p = nullptr;
    bool redo = false;

    if (order >= MAX_PAGE_ORDER) {
        return nullptr;
    }

    this->lock.Lock();

redo:
    /* try to alloc directly */
    p = this->__alloc_page_direct(order);
    if (p) {
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

auto PagePool::__free_pages(Page *p, base::size_t order) -> void
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
        Page *buddy;

        buddy = get_page_buddy(p, order);
        if (buddy->type == PAGE_NORMAL_MEM && buddy->is_head && buddy->is_free) {
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

    this->__reinit_page(p, order, true);

    list_add_next(&(this->freelist[order]), &p->list);

    this->lock.UnLock();
}

auto PagePool::__reclaim_memory(void) -> void
{
    /* TODO: reclame movable memory pages */
}

auto PagePool::AllocPages(base::size_t order) -> Page *
{
    return this->__alloc_pages(order);
}

auto PagePool::FreePages(Page *page, base::size_t order) -> void
{
    this->__free_pages(page, order);
}

auto PagePool::Init(void) -> void
{
    for (auto i = 0; i < MAX_PAGE_ORDER; i++) {
        lib::list_head_init(&this->freelist[i]);
    }

    this->lock.Reset();
}

auto PagePool::AddPages(Page *page, base::size_t order) -> void
{
    for (auto i = 0; i < (1 << order); i++) {
        page->pool = this;  /* shoudl NOT be changed after initialization */
    }

    this->FreePages(page, order);
}

};
