#include <boot/multiboot2.h>

extern void boot_puts(const char *str);
extern void mm_core_init(void);

void main(multiboot_uint8_t *mbi)
{
    mm_core_init();
    /* do nothing */

    while (1) {
        boot_puts("[x] No work todo, hlting...");
        asm volatile ("hlt");
    }
}
