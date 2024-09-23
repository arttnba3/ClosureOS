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
    GloblPagePool->Reset();

    for (base::size_t i = 0; i < pgdb_page_nr; i++) {
        if (pgdb_base[i].type == PAGE_NORMAL_MEM && pgdb_base[i].ref_count < 0){
            GloblPagePool->FreePages(&pgdb_base[i], 0);
        }
    }
}

auto mm_core_init(void) -> void
{
    pages_pool_init();
}

};
