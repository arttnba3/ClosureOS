#include <closureos/mm/pages.h>
#include <closureos/mm/mm_types.h>
#include <closureos/lock.h>

struct page *pgdb_base;
size_t pgdb_page_nr;
spinlock_t pgdb_lock;

/* the `core` of the buddy system, which should be a cycle linked-list */
struct list_head freelist[MAX_PAGE_ORDER];

virt_addr_t physmem_base, vmremap_base, kernel_base;

struct page *next_page_by_order(struct page *p, int order)
{
    if (order >= MAX_PAGE_ORDER) {
        return NULL;
    }

    return p + (1 << order);
}

struct page *prev_page_by_order(struct page *p, int order)
{
    if (order >= MAX_PAGE_ORDER) {
        return NULL;
    }

    return p - (1 << order);
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
                          &next_page_by_order(p, allocated)->list);
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
        p->ref_count++;
        p->is_head = true;
        p->order = order;
        p->is_free = false;

        for (int i = 1; i < ((1 << order) - 1); i++) {
            p[i].is_head = false;
            p[i].order = order;
        }

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
        struct page *next, *prev;

        next = next_page_by_order(p, order);
        if (next->type == PAGE_NORMAL_MEM && next->is_free && next->is_head) {
            list_del(&next->list);
            next->is_head = false;
            order++;
            continue;
        }

        prev = prev_page_by_order(p, order);
        if (prev->type == PAGE_NORMAL_MEM && prev->is_free && prev->is_head) {
            list_del(&prev->list);
            p->is_head = false;
            p->is_free = true;
            order++;
            p = prev;
            continue;
        }

        /* we can't combine forward or backward, just break */
        break;
    }

    list_add_next(&(freelist[order]), &p->list);
    p->ref_count = p->map_count = -1;
    p->is_free = p->is_head = true;

    spin_unlock(&pgdb_lock);
}

void free_pages(struct page *p, int order)
{
    __free_pages(p, order);
}
