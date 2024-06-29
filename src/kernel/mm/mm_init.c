#include <closureos/mm/pages.h>
#include <closureos/list.h>
#include <closureos/lock.h>
#include <closureos/mm/slub.h>

void freelist_init(void)
{
    for (int i = 0;i < MAX_PAGE_ORDER; i++) {
        list_head_init(&freelist[i]);
    }
}

static void page_database_init(void)
{
    spin_lock_init(&pgdb_lock);

    int free_count = 0;

    for (size_t i = 0; i < pgdb_page_nr; i++) {
        if (pgdb_base[i].type == PAGE_NORMAL_MEM && pgdb_base[i].ref_count < 0){
            free_pages(&pgdb_base[i], 0);
        }
    }
}

void mm_core_init(void)
{
    freelist_init();
    page_database_init();
    kmem_cache_init();
}
