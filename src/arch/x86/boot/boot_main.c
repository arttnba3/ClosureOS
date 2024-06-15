#include <boot/multiboot2.h>
#include <asm/page_types.h>
#include <closureos/types.h>
#include <boot/tty.h>

extern uint64_t boot_pud[512];

/**
 * map 512 GB phys memory directly for booting stage
 * As it's troublesome to do it in assembly, I'd rather map only 1 GB in that,
 * and fulfill the left part in the PUD page here.
*/
void boot_pgtable_init(void)
{
    for (uint64_t i = 1; i < 512; i++) {
        boot_pud[i] = (i*0x40000000) | (PAGE_ATTR_P|PAGE_ATTR_RW|PAGE_ATTR_PS);
    }
}

void boot_main(unsigned int magic, multiboot_uint8_t *mbi)
{
    boot_pgtable_init();
    
    if (boot_tty_init(mbi) < 0) {
        asm volatile ("hlt");
    }

    while (1) {
        asm volatile ("hlt");
    }
}
