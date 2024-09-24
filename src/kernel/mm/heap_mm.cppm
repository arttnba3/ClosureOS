export module kernel.mm:heap;

import :pages;
import :types;
import kernel.base;
import kernel.lib;

export namespace mm {

/* we can only allocate max 8 pages at a time */
inline constexpr base::size_t KMALLOC_MAX_SHIFT = 1;
inline constexpr base::size_t KMALLOC_MAX_CACHE_SIZE = (PAGE_SIZE << KMALLOC_MAX_SHIFT);

enum {
    KOBJECT_16 = 0,
    KOBJECT_32,
    KOBJECT_64,
    KOBJECT_128,
    KOBJECT_192,
    KOBJECT_256,
    KOBJECT_512,
    KOBJECT_1K,
    KOBJECT_2K,
    KOBJECT_4K,
    KOBJECT_8K, /* max allocation for 2 pages */
    KOBJECT_SIZE_NR,
};

base::size_t kobj_default_size[KOBJECT_SIZE_NR] = {
    16,
    32,
    64,
    128,
    192,
    256,
    512,
    1024,
    2048,
    4096,
    8192,
};

inline constexpr base::size_t CACHE_POOL_MAX_NR = 0x10;

/* size-specific memory pool, front end of PagePool */
class KMemCache {
public:
    KMemCache(void);
    ~KMemCache();

    auto Malloc(void) -> void*;
    auto Free(Page *page, void *obj) -> void;

    auto AddPool(PagePool *pool) -> bool;
    auto RemovePool(base::size_t index) -> PagePool*;

    auto Init(base::size_t obj_sz) -> void;

private:
    /* pages pool backend */

    PagePool *pools[CACHE_POOL_MAX_NR];
    base::size_t pool_nr;
    base::size_t order;

    auto __internal_page_alloc(void) -> Page*;
    auto __page_obj_slicing(Page* page) -> void;

    /* caches front end */

    base::size_t page_obj_nr;
    base::size_t obj_sz;
    Page *current;
    lib::ListHead partial;
    lib::ListHead full;
    void **freelist;

    auto __internal_obj_alloc(void) -> void*;
    auto __internal_obj_free(Page *page, void *obj) -> void;

    /* infrastructure */

    lib::atomic::SpinLock lock;
};

/* static memory initializer to avoid constructor to be existed */
base::uint8_t GloblKMemCacheGroupMem[KOBJECT_SIZE_NR][sizeof(KMemCache)];
KMemCache *GloblKMemCacheGroup[KOBJECT_SIZE_NR] = {
    (KMemCache*) &GloblKMemCacheGroupMem[KOBJECT_16],
    (KMemCache*) &GloblKMemCacheGroupMem[KOBJECT_32],
    (KMemCache*) &GloblKMemCacheGroupMem[KOBJECT_64],
    (KMemCache*) &GloblKMemCacheGroupMem[KOBJECT_128],
    (KMemCache*) &GloblKMemCacheGroupMem[KOBJECT_192],
    (KMemCache*) &GloblKMemCacheGroupMem[KOBJECT_256],
    (KMemCache*) &GloblKMemCacheGroupMem[KOBJECT_512],
    (KMemCache*) &GloblKMemCacheGroupMem[KOBJECT_1K],
    (KMemCache*) &GloblKMemCacheGroupMem[KOBJECT_2K],
    (KMemCache*) &GloblKMemCacheGroupMem[KOBJECT_4K],
    (KMemCache*) &GloblKMemCacheGroupMem[KOBJECT_8K],
};

KMemCache::KMemCache(void)
{
    /* do nothing */
}

KMemCache::~KMemCache(void)
{
    /* do nothing */
}

auto KMemCache::Malloc(void) -> void*
{
    void *obj;

    this->lock.Lock();

    obj = this->__internal_obj_alloc();

    this->lock.UnLock();

    return obj;
}

auto KMemCache::Free(Page *page, void *obj) -> void
{
    this->lock.Lock();

    this->__internal_obj_free(page, obj);

    this->lock.UnLock();
}

auto KMemCache::AddPool(PagePool *pool) -> bool
{
    PagePool **slot = nullptr;

    if (this->pool_nr == CACHE_POOL_MAX_NR) {
        return false;
    }

    for (auto i = 0; i < CACHE_POOL_MAX_NR; i++) {
        if (!this->pools[i]) {
            slot = &this->pools[i];
            break;
        }
    }

    if (!slot) {
        return false;
    }

    *slot = pool;

    return true;
}

auto KMemCache::RemovePool(base::size_t index) -> PagePool*
{
    PagePool *candidate = nullptr;

    if (index >= CACHE_POOL_MAX_NR) {
        return nullptr;
    }

    if (this->pools[index]) {
        candidate = this->pools[index];
        this->pools[index] = nullptr;
    }

    return candidate;
}

auto KMemCache::Init(base::size_t obj_sz) -> void
{
    for (auto i = 0; i < CACHE_POOL_MAX_NR; i++) {
        this->pools[i] = nullptr;
    }

    this->pool_nr = 0;
    this->order = obj_sz >> PAGE_SHIFT;
    if (obj_sz >= PAGE_SIZE) {
        this->order++;
    }

    this->obj_sz = obj_sz;
    this->page_obj_nr = (PAGE_SIZE << this->order) / this->obj_sz;

    this->current = nullptr;
    lib::list_head_init(&this->partial);
    lib::list_head_init(&this->full);
    this->freelist = nullptr;

    this->lock.Reset();
}

auto KMemCache::__internal_page_alloc(void) -> Page*
{
    Page *new_page = nullptr;

    for (auto i = 0; i < CACHE_POOL_MAX_NR; i++) {
        if (this->pools[i]) {
            new_page = this->pools[i]->AllocPages(this->order);
            if (new_page) {
                break;
            }
        }
    }

    return new_page;
}

auto KMemCache::__page_obj_slicing(Page* page) -> void
{
    void *curr_obj;
    virt_addr_t next_obj;
    void **obj_ptr;

    next_obj = page_to_virt(page);
    curr_obj = nullptr;
    obj_ptr = (void**) next_obj;

    for (auto i = 0; i < this->page_obj_nr; i++) {
        *obj_ptr = curr_obj;
        curr_obj = (void*) next_obj;
        next_obj += this->obj_sz;
        obj_ptr = (void**) next_obj;
    }

    page->obj_nr = this->page_obj_nr;
    page->freelist = (void**) curr_obj;
    page->kc = this;
    get_page(page);

}

auto KMemCache::__internal_obj_alloc(void) -> void*
{
    void *obj = nullptr;

redo:
    /* we have objects on the kmem_cache now, just allocate one */
    if (this->freelist != nullptr) {
        obj = this->freelist;
        this->freelist = (void**) (*this->freelist);
        this->current->obj_nr--;
        goto out;
    }

    /* no object on freelist, put the current page on the full list */
    if (this->current) {
        lib::list_add_next(&this->full, &this->current->list);
        this->current = nullptr;
    }

    /* try to get the page from partial list */
    if (!lib::list_empty(&this->partial)) {
        this->current = lib::container_of(this->partial.next, &Page::list);
        this->freelist = this->current->freelist;
        lib::list_del(&this->current->list);
        goto redo;
    }

    /* no current on the partial list, allocated from the buddy */
    this->current = this->__internal_page_alloc();
    if (this->current) {
        this->__page_obj_slicing(this->current);
        this->freelist = this->current->freelist;
        goto redo;
    }

out:
    return obj;
}

auto KMemCache::__internal_obj_free(Page *page, void *obj) -> void
{
    page->obj_nr++;

    if (page == this->current) {
        *(void**) obj = this->freelist;
        this->freelist = (void**) obj;
    } else {
        if (page->obj_nr == 1) {    /* on full list */
            list_del(&page->list);
            list_add_next(&this->full, &page->list);
        } else if (page->obj_nr == this->page_obj_nr) {   /* all freed */
            list_del(&page->list);
            put_page(page);
        }
    }
}

/* General front end of KMemCache */
class KHeapPool {
public:
    KHeapPool(void);
    ~KHeapPool();

    auto Malloc(base::size_t sz) -> void*;
    auto Free(void *obj) -> void;

    auto Init(void) -> void;

private:
    KMemCache **caches;
    base::size_t cache_nr;
    base::size_t *cache_obj_sz;
};

static base::uint8_t GloblKHeapPoolMem[sizeof(KHeapPool)];
KHeapPool *GloblKHeapPool = (KHeapPool*) &GloblKHeapPoolMem;

KHeapPool::KHeapPool(void)
{
    /* do nothing */    
}

KHeapPool::~KHeapPool()
{
    /* do nothing */
}

auto KHeapPool::Malloc(base::size_t sz) -> void*
{
    void *obj = nullptr;

    for (auto i = 0; i < this->cache_nr; i++) {
        if (sz < this->cache_obj_sz[i]) {
            obj = this->caches[i]->Malloc();
            if (obj) {
                break;
            }
        }
    }

    /* TODO: add large size alloc */

    return obj;
}

auto KHeapPool::Free(void *obj) -> void
{
    Page *page;

    page = get_head_page(virt_to_page((virt_addr_t) obj));
    if (!page->kc) { /* TODO: add initializer for kc */
        put_page(page);
        return ;
    }

    page->kc->Free(page, obj);
}

auto KHeapPool::Init(void) -> void
{

}

};
