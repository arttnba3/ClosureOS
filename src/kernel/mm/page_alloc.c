#include <closureos/mm/pages.h>
#include <closureos/mm/mm_types.h>
#include <closureos/lock.h>

struct page *pgdb_base;
size_t pgdb_page_nr;
spinlock_t pgdb_lock;

/* the `core` of the buddy system, which should be a cycle linked-list */
struct list_head freelist[MAX_PAGE_ORDER];

virt_addr_t physmem_base, vmremap_base, kernel_base;

static __always_inline size_t buddy_page_pfn(pfn_t pfn, int order)
{
    return pfn ^ (1 << order);
}

static __always_inline struct page* get_page_buddy(struct page *p, int order)
{
    pfn_t pfn = page_to_pfn(p);
    return pfn_to_page(buddy_page_pfn(pfn, order));
}

/**
 * @brief allocate pages directly from the freelist array.
 * 
 * @param order the order of memory allocation
 * @return struct page* pointer to the page struct, NULL for failed
 */
static struct page *__alloc_page_direct(int order)
{
    struct page *p = NULL;
    int allocated = order;

    while (allocated < MAX_PAGE_ORDER) {
        if (!list_empty(&freelist[allocated])) {
            p = list_entry(freelist[allocated].next, struct page, list);
            list_del(&p->list);
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
            list_add_next(&freelist[allocated],
                          &get_page_buddy(p, allocated)->list);
            allocated--;
        }
    }

out:
    return p;
}

/* try to find movable pages and collect pieces */
static void __reclaim_memory(void)
{

}

#include <boot/tty.h>

/* This is the `heart` of buddy system allocation */
static struct page *__alloc_pages(int order)
{
    struct page *p = NULL;
    bool redo = false;

    if (order >= MAX_PAGE_ORDER) {
        return NULL;
    }

    spin_lock(&pgdb_lock);

redo:
    /* try to alloc directly */
    p = __alloc_page_direct(order);
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
        __reclaim_memory();
        redo = true;
        goto redo;
    }

out:
    spin_unlock(&pgdb_lock);

    return p;
}

struct page *alloc_pages(int order)
{
    struct page *p;

    p = __alloc_pages(order);

    return p;
}

/* This is the `heart` of buddy system free */
static void __free_pages(struct page *p, int order)
{
    if (!p) {
        return;
    }

    if (order >= MAX_PAGE_ORDER) {
        return;
    }

    spin_lock(&pgdb_lock);

    /* try to combine nearby pages */
    while (order < (MAX_PAGE_ORDER - 1)) {
        struct page *buddy;

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

    list_add_next(&(freelist[order]), &p->list);

    spin_unlock(&pgdb_lock);
}

void free_pages(struct page *p, int order)
{
    __free_pages(p, order);
}
