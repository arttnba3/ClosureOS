#include <closureos/types.h>
#include <boot/multiboot2.h>
#include <closureos/lock.h>
#include <closureos/cpp_base.hpp>

extern "C" void boot_puts(const char *str);
extern "C" void mm_core_init(void);

auto global_constructor_caller(void) -> int
{
    int (**init_array)(void) = &__init_array;
    int error;

    for (size_t i = 0; init_array[i]; i++) {
        error = init_array[i]();
        if (error) {
            return error;
        }
    }

    spin_lock_init(&dtor_exit_lock);

    return 0;
}

extern "C" auto main(multiboot_uint8_t *mbi) -> void
{
    mm_core_init();
    /* do nothing */

    if (global_constructor_caller() < 0) {
        boot_puts("[x] FAILED at invoking global constructors, hlting...");
        asm volatile ("hlt");
    }

    while (1) {
        boot_puts("[x] No work todo, hlting...");
        asm volatile ("hlt");
    }
}
