#ifndef MM_SLUB_H
#define MM_SLUB_H

#include <closureos/types.h>
#include <closureos/list.h>
#include <closureos/lock.h>

/* we can only allocate max 8 pages from slub at a time */
#define KMALLOC_MAX_SHIFT 1
#define KMALLOC_MAX_CACHE_SIZE (PAGE_SIZE << KMALLOC_MAX_SHIFT)

struct kmem_cache {
    spinlock_t lock;
    size_t obj_size;
    size_t slub_obj_nr;
    size_t slub_order;
    struct list_head full;      /* list of full pages */
    struct list_head partial;   /* list of partial pages */
    struct list_head list;      /* list of slub caches */
    struct page *slub;          /* current slub page */
    void **freelist;            /* current freelist */
};

enum {
    KOBJECT_16 = 0,
    KOBJECT_32,
    KOBJECT_64,
    KOBJECT_128,
    KOBJECT_256,
    KOBJECT_512,
    KOBJECT_1K,
    KOBJECT_2K,
    KOBJECT_4K,
    KOBJECT_8K, /* max allocation for 2 pages */
    NR_KOBJECT_SIZE_TYPE,
};

extern struct kmem_cache kmem_cache_list[NR_KOBJECT_SIZE_TYPE];

extern void *kmalloc(size_t sz);
extern void kfree(void* object);
extern void kmem_cache_init(void);

#endif // MM_SLUB_H
