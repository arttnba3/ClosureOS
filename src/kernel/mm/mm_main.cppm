export module kernel.mm;
export import :heap;
export import :layout;
export import :pages;
export import :types;

import kernel.base;
import kernel.lib;

export namespace mm {

static auto pages_pool_init(void) -> void
{
    GloblPagePool->Init();

    for (auto i = 0; i < pgdb_page_nr; i++) {
        if (pgdb_base[i].type == PAGE_NORMAL_MEM && pgdb_base[i].ref_count < 0) {
            GloblPagePool->AddPages(&pgdb_base[i], 0);
        }
    }
}

static auto kheap_pool_init(void) -> void
{
    for (auto i = 0; i < KOBJECT_SIZE_NR; i++) {
        GloblKMemCacheGroup[i]->Init(kobj_default_size[i]);
        GloblKMemCacheGroup[i]->AddPool(GloblPagePool);
    }

    GloblKHeapPool->Init();
    GloblKHeapPool->SetKMemCaches(GloblKMemCacheGroup, KOBJECT_SIZE_NR, kobj_default_size);
    GloblKHeapPool->SetPagePools(GloblPagePoolGroup, PAGE_POOL_TYPE_NR);
}

auto mm_core_init(void) -> void
{
    pages_pool_init();
    kheap_pool_init();
}

};
