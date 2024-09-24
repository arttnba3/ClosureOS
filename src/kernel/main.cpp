import kernel.base;
import kernel.lib;
import kernel.mm;

#include <boot/multiboot2.h>
#include <closureos/cpp_base.hpp>

/* temporarily here, remove sooo... */
extern "C" {
#include <boot/tty.h>
};

auto global_constructor_caller(void) -> int
{
    int (**init_array)(void) = &__init_array;
    int error;

    for (base::size_t i = 0; init_array[i]; i++) {
        boot_printstr("func to be called: 0x");
        boot_printhex((base::uint64_t) (init_array[i]));
        boot_putchar('\n');
        error = init_array[i]();
        if (error) {
            boot_printstr("Error code: ");
            boot_printnum(error);
            boot_putchar('\n');
            return error;
        }
    }

    return 0;
}

extern "C" auto main(multiboot_uint8_t *mbi) -> void
{
    mm::mm_core_init();

    if (global_constructor_caller() < 0) {
        boot_puts("[x] FAILED at invoking global constructors, hlting...");
        asm volatile ("hlt");
    }

    while (1) {
        boot_puts("[x] No work todo, hlting...");
        asm volatile ("hlt");
    }
}
