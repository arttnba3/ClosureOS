#ifndef MM_PAGES_H
#define MM_PAGES_H

#include <closureos/types.h>
#include <closureos/list.h>
#include <asm/page_types.h>

enum {
    PAGE_NON_EXISTED = 0,
    PAGE_NORMAL_MEM,
    PAGE_RESERVED,
    PAGE_ACPI_RECLAIMABLE,
    PAGE_NVS,
    PAGE_BADRAM,
};

/**
 * struct page
 * - representing a physical page frame
 * - page-aligned size
 */
struct page {
    struct list_head list;
    struct {
        /* for page allocator */
        unsigned type: 8;
        unsigned is_free: 1; /* already in freelist */
        unsigned is_head: 1; /* head of a group of pages*/
        unsigned order: 4;
    };
    int ref_count;
    int map_count;
    void **freelist;    /* used only when the slub is not in pool */
    struct kmem_cache *kc;  /* used only when it's a slub page */
    size_t obj_nr;

    /* unused area to make it page-aligned, maybe we can put sth else there? */
    size_t unused[0];
} __attribute__((aligned(16)));

/* pages array representing all pages */
extern struct page *pgdb_base;
extern size_t pgdb_page_nr;

/* number of `struct page` in a page frame */
#define PGDB_PG_PAGE_NR (PAGE_SIZE / sizeof(struct page))

#endif 
