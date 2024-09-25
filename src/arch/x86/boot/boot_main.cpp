extern "C" {

#include <boot/multiboot2.h>
#include <asm/page_types.h>
#include <boot/tty.h>

extern uint64_t boot_pud[512];

};

extern auto boot_mm_init(multiboot_uint8_t *mbi) -> int;
extern auto main(multiboot_uint8_t *mbi) -> void;

/**
 * map 512 GB phys memory directly for booting stage
 * As it's troublesome to do it in assembly, I'd rather map only 1 GB in that,
 * and fulfill the left part in the PUD page here.
*/
static auto boot_pgtable_init(void) -> void
{
    for (uint64_t i = 1; i < 512; i++) {
        boot_pud[i] = (i*0x40000000) | (PDE_ATTR_P | PDE_ATTR_RW | PDE_ATTR_PS);
    }
}

extern "C" void boot_main(unsigned int magic, multiboot_uint8_t *mbi)
{
    int ret;

    boot_pgtable_init();

    if (boot_tty_init(mbi) < 0) {
        asm volatile ("hlt");
    }

    boot_puts("[+] booting-stage tty initialization done.");

    if ((ret = boot_mm_init(mbi)) < 0) {
        boot_printstr("[x] FAILED to initialize memory unit, errno: ");
        boot_printnum(ret);
        boot_puts("\n[!] Abort booting.");
        asm volatile ("hlt");
    }

    boot_puts("[+] booting-state memory initialization done.");

    main(mbi);
}
