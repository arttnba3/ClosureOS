#ifndef X86_BOOT_TTY_H
#define X86_BOOT_TTY_H

#include <closureos/types.h>
#include <boot/multiboot2.h>

extern uint32_t *boot_fb_base, *boot_fb_end;
extern uint32_t boot_fb_width, boot_fb_height;

extern bool boot_tty_has_fb(void);
extern bool boot_tty_has_com(void);

extern size_t boot_tty_fb_sz(void);

extern void boot_putchar(uint16_t ch);
extern void boot_printstr(const char *str);
extern void boot_puts(const char *str);
extern void boot_printnum(int64_t n);
extern void boot_printhex(uint64_t n);

extern int boot_tty_init(multiboot_uint8_t *mbi);

#endif // X86_BOOT_TTY_H
