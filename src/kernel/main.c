#include <closureos/types.h>
#include <boot/multiboot2.h>
#include <closureos/lock.h>

extern void boot_puts(const char *str);
extern void mm_core_init(void);

extern void (*__init_array)(void);
extern spinlock_t dtor_exit_lock;

void global_constructor_caller(void)
{
    void (**init_array)(void) =&__init_array;

    for (size_t i = 0; init_array[i]; i++) {
        init_array[i]();
    }

    spin_lock_init(&dtor_exit_lock);
}

void main(multiboot_uint8_t *mbi)
{
    mm_core_init();
    /* do nothing */

    global_constructor_caller();

    while (1) {
        boot_puts("[x] No work todo, hlting...");
        asm volatile ("hlt");
    }
}
