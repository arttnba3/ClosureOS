#include <closureos/types.h>
#include <closureos/mm/slub.h>

struct kmem_cache kmem_cache_list[NR_KOBJECT_SIZE_TYPE];

void *kmalloc(size_t sz)
{
    return NULL;
}

void kfree(void* object)
{

}

void kmem_cache_init(void)
{

}
