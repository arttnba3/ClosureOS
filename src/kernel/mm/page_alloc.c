#include <closureos/types.h>
#include <closureos/mm/mm_types.h>
#include <closureos/lock.h>

struct page *pgdb_base;
size_t pgdb_page_nr;
spinlock_t pgdb_lock;
