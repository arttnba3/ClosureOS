#include <closureos/types.h>
#include <closureos/mm/slub.h>
#include <closureos/err.h>

struct kmem_cache kmem_cache_list[NR_KOBJECT_SIZE];
size_t kmem_cache_obj_sz[NR_KOBJECT_SIZE] = {
        16, 32, 64, 128,
        192, 256, 512, 1024,
        2048, 4096, 8192
};

static __always_inline int slub_index(size_t sz)
{
    for (int i = 0; i < NR_KOBJECT_SIZE; i++) {
        if (sz <= kmem_cache_obj_sz[i]) {
            return i;
        }
    }

    return -E2BIG;
}

static struct page* slub_alloc(struct kmem_cache *kc)
{
    struct page *new_slub;
    void *curr_obj;
    virt_addr_t next_obj;
    void **obj_ptr;

    new_slub = alloc_pages(kc->slub_order);

    /* trim pages into chunks */
    if (new_slub) {
        next_obj = page_to_virt(new_slub);
        curr_obj = NULL;
        obj_ptr = (void**) next_obj;

        for (size_t i = 0; i < kc->slub_obj_nr; i++) {
            *obj_ptr = curr_obj;
            curr_obj = (void*) next_obj;
            next_obj += kc->obj_size;
            obj_ptr = (void**) next_obj;
        }

        new_slub->obj_nr = kc->slub_obj_nr;
        new_slub->freelist = curr_obj;
        new_slub->kc = kc;
        get_page(new_slub);
    }

    return new_slub;
}

/**
 * @brief Fetch an object from a specific kmem_cache
 *
 * @return void* allocated object, NULL for failure
 */
static void* __kmem_cache_alloc(struct kmem_cache *kc)
{
    void *obj = NULL;

    if (!kc) {
        return NULL;
    }

    spin_lock(&kc->lock);

redo:
    /* we have objects on the kmem_cache now, just allocate one */
    if (kc->freelist != NULL) {
        obj = kc->freelist;
        kc->freelist = *kc->freelist;
        kc->slub->obj_nr--;
        goto out;
    }

    /* no object on freelist, put the slub page on the full list */
    if (kc->slub) {
        list_add_next(&kc->full, &kc->slub->list);
        kc->slub = NULL;
    }

    /* try to get the page from partial list */
    if (!list_empty(&kc->partial)) {
        kc->slub = container_of(kc->partial.next, struct page, list);
        kc->freelist = kc->slub->freelist;
        list_del(&kc->slub->list);
        goto redo;
    }

    /* no slub on the partial list, allocated from the buddy */
    kc->slub = slub_alloc(kc);
    if (kc->slub) {
        kc->freelist = kc->slub->freelist;
        goto redo;
    }

out:
    spin_unlock(&kc->lock);

    return obj;
}

static void* __kmalloc_normal(size_t sz)
{
    int slub_idx;

    slub_idx = slub_index(sz);
    if (slub_idx < 0) {
        return NULL;
    }

    return __kmem_cache_alloc(&kmem_cache_list[slub_idx]);
}

static void* __kmalloc_large(size_t sz)
{
    struct page *p;

    p = alloc_pages(sz >> (PAGE_SHIFT + 1));
    if (!p) {
        /* failed to allocate */
        return NULL;
    }
    get_page(p);

    return (void*) page_to_virt(p);
}

void* kmalloc(size_t sz)
{
    if (sz <= KMALLOC_MAX_CACHE_SIZE) {
        return __kmalloc_normal(sz);
    }

    return __kmalloc_large(sz);
}

static void
__kmem_cache_free(struct kmem_cache *kc, struct page *slub, void *obj)
{
    spin_lock(&kc->lock);

    kc->slub->obj_nr++;

    if (slub == kc->slub) {
        *(void**) obj = kc->freelist;
        kc->freelist = obj;
    } else {
        if (slub->obj_nr == 1) {    /* on full list */
            list_del(&slub->list);
            list_add_next(&kc->full, &slub->list);
        } else if (slub->obj_nr == kc->slub_obj_nr) {   /* all freed */
            list_del(&slub->list);
            put_page(slub);
        }
    }

    spin_unlock(&kc->lock);
}

void kfree(void* object)
{
    struct page *p;

    if (!object) {
        return;
    }

    p = get_head_page(virt_to_page((virt_addr_t) object));
    if (!p->kc) {
        put_page(p);
        return;
    }

    __kmem_cache_free(p->kc, p, object);
}

void kmem_cache_init(void)
{
    for (int i = 0; i < NR_KOBJECT_SIZE; i++) {
        struct kmem_cache *kc = &kmem_cache_list[i];

        /* list for partial and full used pages */
        list_head_init(&kc->partial);
        list_head_init(&kc->full);

        /* list for current allocation */
        kc->freelist = NULL;
        kc->slub = NULL;

        /* size for each kmem_cache, times 2 each time */
        kc->obj_size = kmem_cache_obj_sz[i];
        if (kc->obj_size < PAGE_SIZE) {
            kc->slub_order = (kc->obj_size >> PAGE_SHIFT);
        } else {
            kc->slub_order = (kc->obj_size >> PAGE_SHIFT) + 1;
        }
        kc->slub_obj_nr = (PAGE_SIZE << kc->slub_order) / kc->obj_size;

        /* other variables */
        spin_lock_init(&kc->lock);
    }
}
