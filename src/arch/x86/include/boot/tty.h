#ifndef X86_BOOT_TTY_H
#define X86_BOOT_TTY_H

#include <closureos/types.h>
#include <boot/multiboot2.h>

extern void boot_putchar(uint16_t ch);
extern void boot_printstr(const char *str);
extern void boot_puts(const char *str);
extern void boot_printnum(int64_t n);
extern void boot_printhex(uint64_t n);
extern int boot_tty_init(multiboot_uint8_t *mbi);

#endif // X86_BOOT_TTY_H
