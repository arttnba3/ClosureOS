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
