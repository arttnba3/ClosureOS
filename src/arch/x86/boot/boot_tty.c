/**
 * Boot stage operations related to tty and graphic output.
 * 
 * Copyright (c) 2024 arttnba3 <arttnba@gmail.com>
 * 
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
*/

#include <closureos/types.h>
#include <boot/multiboot2.h>
#include <graphics/tty/font/psf.h>
#include <graphics/tty/default.h>
#include <asm/com.h>
#include <asm/io.h>

/**
 * Frame Buffer font output
*/

extern char _binary_font_psf_start[];
extern char _binary_font_psf_end[];

/* frame buffer info */

static uint32_t *framebuffer_base; /* 32 bit color */
static uint32_t framebuffer_width;
static uint32_t framebuffer_height;

/* char output info */

static uint32_t fb_cursor_x, fb_cursor_y;    /* count by chars */
static uint32_t max_ch_nr_x, max_ch_nr_y;

/* font info */

static struct psf2_header *boot_font;
static uint32_t font_width, font_height;
static uint32_t font_width_bytes;
static uint8_t *glyph_table;
static uint32_t bytes_per_glyph, glyph_nr;

static void boot_putchar_fb_new_line(uint32_t bg)
{
    fb_cursor_x = 0;
    fb_cursor_y++;
    
    /* we may need to scroll up */
    if (fb_cursor_y >= max_ch_nr_y) {
        for (uint32_t y = 0; y < ((max_ch_nr_y - 1) * font_height); y++) {
            for (uint32_t x = 0; x < framebuffer_width; x++) {
                framebuffer_base[x + y * framebuffer_width] = 
                framebuffer_base[x + (y + font_height) * framebuffer_width];
            }
        }

        for (uint32_t y = 0; y < font_height; y++) {
            for (uint32_t x = 0; x < framebuffer_width; x++) {
                size_t lines = (y + (max_ch_nr_y - 1) * font_height);
                size_t loc = lines * framebuffer_width + x;
                framebuffer_base[loc] = bg;
            }
        }

        fb_cursor_y--;
    }
}

static void boot_putchar_fb(uint16_t ch, uint32_t fg, uint32_t bg)
{
    uint8_t *glyph = glyph_table;
    size_t loc;

    /* if char out of range, output null */
    if (ch < glyph_nr) {
        glyph += ch * bytes_per_glyph;
    }

    loc =  fb_cursor_y * font_height * framebuffer_width;
    loc += fb_cursor_x * font_width;

    /* output the font to frame buffer */
    for (uint32_t ch_y = 0; ch_y < font_height; ch_y++) {
        uint8_t mask = 1 << 7;

        for (uint32_t ch_x = 0; ch_x < font_width; ch_x++) {
            if ((*(glyph + ch_x / 8) & mask) != 0) {
                framebuffer_base[loc + ch_y * framebuffer_width + ch_x] = fg;
            } else {
                framebuffer_base[loc + ch_y * framebuffer_width + ch_x] = bg;
            }

            mask >>= 1;
            if (ch_x % 8 == 0) {
                mask = 1 << 7;
            }
        }

        glyph += font_width_bytes;
    }

    /* move cursor */
    fb_cursor_x++;

    /* we may need to move to new line */
    if (fb_cursor_x >= max_ch_nr_x) {
        boot_putchar_fb_new_line(bg);
    }
}

static void boot_clear_screen_internal_fb(void)
{
    for (uint32_t y = 0; y < framebuffer_height; y++) {
        for (uint32_t x = 0; x < framebuffer_width; x++) {
            framebuffer_base[y * framebuffer_width + x] = 0;
        }
    }
}

static int boot_get_frame_buffer(multiboot_uint8_t *mbi)
{
    struct multiboot_tag_framebuffer *fb_info = NULL;
    struct multiboot_tag *tag = (struct multiboot_tag *) (mbi + 8);

    /* find framebuffer tag */
    if (tag == NULL) {
        return -1;
    }

    while (tag->type != MULTIBOOT_TAG_TYPE_END) {
        if (tag->type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER) {
            fb_info = (struct multiboot_tag_framebuffer*) tag;
            break;
        }

        tag = (struct multiboot_tag *) \
              ((multiboot_uint8_t *) tag + ((tag->size + 7) & ~7));
    }

    if (fb_info == NULL) {
        return -1;
    }

    framebuffer_base = (uint32_t*) fb_info->common.framebuffer_addr;
    framebuffer_height = fb_info->common.framebuffer_height;
    framebuffer_width = fb_info->common.framebuffer_width;

    return 0;
}

static void boot_cursor_back_fb(void)
{
    fb_cursor_x = 0;
}

static void boot_back_space_fb(void)
{
    if (fb_cursor_x > 0) {
        fb_cursor_x--;
    }
}

static void boot_putchar_tab_fb(void)
{
    int left_ch_nr, space_nr;

    left_ch_nr = max_ch_nr_x - fb_cursor_x - 1;
    space_nr = (left_ch_nr > DEFAULT_TAB_SIZE)
                ? DEFAULT_TAB_SIZE
                : left_ch_nr;

    for (int i = 0; i < space_nr; i++) {
        boot_putchar_fb(' ', 0xffffff, 0);
    }
}

static void boot_init_font(void)
{
    boot_font = (struct psf2_header*) _binary_font_psf_start;

    font_width_bytes = (boot_font->width + 7) / 8;
    font_width = font_width_bytes * 8;
    font_height = boot_font->height;

    glyph_table = (uint8_t*)_binary_font_psf_start+boot_font->header_size;
    glyph_nr = boot_font->glyph_nr;
    bytes_per_glyph = boot_font->bytes_per_glyph;

    fb_cursor_x = fb_cursor_y = 0;
    max_ch_nr_x = framebuffer_width / font_width;
    max_ch_nr_y = framebuffer_height / font_height;
}

/**
 * Serial Port output
*/

static int com_base = 0;

static void boot_putchar_com(uint16_t ch)
{
    uint8_t res;

    /* wait for the port to be ready */
    do {
        res = inb(com_base + COM_REG_LSR);
    } while ((res & 0x20) == 0);

    outb(com_base, ch);
}

static void boot_cursor_back_com(void)
{
    boot_putchar_com('\r');
}

static void boot_back_space_com(void)
{
    boot_putchar_com('\b');
}

static void boot_putchar_new_line_com(void)
{
    boot_putchar_com('\n');
}

static void boot_putchar_tab_com(void)
{
    boot_putchar_com('\t');
}

static void boot_clear_screen_internal_com(void)
{
    /* ANSI escape: clear screen */
    boot_putchar_com('\x1B');
    boot_putchar_com('[');
    boot_putchar_com('2');
    boot_putchar_com('J');

    /* ANSI escape: cursor back home */
    boot_putchar_com('\x1B');
    boot_putchar_com('[');
    boot_putchar_com('2');
    boot_putchar_com('J');
}

int boot_init_com_internal(size_t base_port)
{
    /* disable all interrupts */
    outb(base_port + COM_REG_IER, 0x00);

    /* enable DLAB to set bound rate divisor */
    outb(base_port + COM_REG_LCR, COM_LCR_DLAB_ON);

    /* set divisor to 38400 baud */
    outb(base_port + COM_REG_DLL, 0x03);
    outb(base_port + COM_REG_DLM, 0x00);

    /* 8 data bits, parity off, 1 stop bit, DLAB latch off */
    outb(base_port + COM_REG_LCR,
         COM_LCR_DB_WLEN_8 | COM_LCR_SB_ONE | COM_LCR_DLAB_OFF);

    /* enable FIFO */
    outb(base_port + COM_REG_FCR,
       COM_FCR_ITLB_TL_14 | COM_FCR_CTFB_ON | COM_FCR_CRFB_ON | COM_FCR_EFB_ON);
    
    /* enable IRQs, set RTS/DSR */
    outb(base_port + COM_REG_MCR,
         COM_MCR_OUT2_ON | COM_MCR_RTSB_ON | COM_MCR_DTRB_ON);

    /* set in loopback mode and test serial chip */
    outb(base_port + COM_REG_MCR,
         COM_MCR_LB_ON | COM_MCR_OUT2_ON | COM_MCR_OUT1_ON | COM_MCR_RTSB_ON);

    /* write a byte to test serial chip */
    outb(base_port + COM_REG_TX, "arttnba3"[0]);

    /* check if serial is faulty */
    if (inb(base_port + COM_REG_RX) != "arttnba3"[0]) {
        return -1;
    }

    /* set in normal mode */
    outb(base_port + COM_REG_MCR,
         COM_MCR_OUT2_ON | COM_MCR_OUT1_ON | COM_MCR_RTSB_ON | COM_MCR_DTRB_ON);

    return 0;
}

int boot_init_com(void)
{
    /* we only try to use two com here */
    if (boot_init_com_internal(COM1_BASE)) {
        if (boot_init_com_internal(COM2_BASE)) {
            return -1;
        } else {
            com_base = COM2_BASE;
        }
    } else {
        com_base = COM1_BASE;
    }

    return 0;
}

/**
 * High-level wrapper
*/

static void boot_cursor_back_no_fb(void)
{
    boot_cursor_back_com();
}

static void boot_cursor_back_no_com(void)
{
    boot_cursor_back_fb();
}

static void boot_cursor_back_default(void)
{
    boot_cursor_back_com();
}

static void (*internal_boot_cursor_back)(void) = boot_cursor_back_default;

static void boot_back_space_no_fb(void)
{
    boot_back_space_com();
}

static void boot_back_space_no_com(void)
{
    boot_back_space_fb();
}

static void boot_back_space_default(void)
{
    boot_back_space_fb();
    boot_back_space_com();
}

static void (*internal_boot_back_space)(void) = boot_back_space_default;

static void boot_putchar_tab_no_fb(void)
{
    boot_putchar_tab_com();
}

static void boot_putchar_tab_no_com(void)
{
    boot_putchar_tab_fb();
}

static void boot_putchar_tab_default(void)
{
    boot_putchar_tab_fb();
    boot_putchar_tab_com();
}

static void (*internal_boot_putchar_tab)(void) = boot_putchar_tab_default;

static void boot_put_new_line_no_fb(void)
{
    boot_putchar_new_line_com();
}

static void boot_put_new_line_no_com(void)
{
    boot_putchar_fb_new_line(0);
}

static void boot_put_new_line_default(void)
{
    boot_putchar_fb_new_line(0);
    boot_putchar_new_line_com();
}

static void (*internal_boot_put_new_line)(void) = boot_put_new_line_default;

static void boot_putchar_no_fb(uint16_t ch)
{
    boot_putchar_com(ch);
}

static void boot_putchar_no_com(uint16_t ch)
{
    boot_putchar_fb(ch, 0xffffff, 0);
}

static void boot_putchar_default(uint16_t ch)
{
    boot_putchar_fb(ch, 0xffffff, 0);
    boot_putchar_com(ch);
}

static void (*internal_boot_putchar)(uint16_t ch) = boot_putchar_default;

void boot_putchar(uint16_t ch)
{
    switch (ch) {
    case '\r':
        internal_boot_cursor_back();
        break;
    case '\b':
        internal_boot_back_space();
        break;
    case '\t':
        internal_boot_putchar_tab();
        break;
    case '\n':
        internal_boot_put_new_line();
        break;
    default:
        internal_boot_putchar(ch);
    }
}

void boot_printstr(const char *str)
{
    while (*str != '\0') {
        boot_putchar(*str);
        str++;
    }
}

void boot_puts(const char *str)
{
    boot_printstr(str);
    boot_putchar('\n');
}

void boot_printnum(int64_t n)
{
    /* for 64-bit num it's enough*/
    char n_str[30];
    int idx = 30;

    n_str[--idx] = '\0';

    if (n < 0) {
        boot_putchar('-');
        n = -n;
    }

    do {
        n_str[--idx] = (n % 10) + '0';
        n /= 10;
    } while (n);
    
    boot_printstr(n_str + idx);
}

void boot_printhex(uint64_t n)
{
    /* for 64-bit num it's enough*/
    char n_str[30];
    int idx = 30;

    n_str[--idx] = '\0';

    do {
        uint64_t curr = n % 16;
        if (curr >= 10) {
            n_str[--idx] = curr - 10 + 'a';
        } else {
            n_str[--idx] = curr + '0';
        }
        n /= 16;
    } while (n);
    
    boot_printstr(n_str + idx);
}

static void boot_clear_screen_default(void)
{
    boot_clear_screen_internal_fb();
    boot_clear_screen_internal_com();
}

static void boot_clear_screen_no_fb(void)
{
    boot_clear_screen_internal_com();
}

static void (*internal_boot_clear_screen)(void) = boot_clear_screen_default;

void boot_clear_screen(void)
{
    internal_boot_clear_screen();
}

/**
 * Initialize for frame buffer and serial port to output.
 * If nothing could be used, return -1.
*/
int boot_tty_init(multiboot_uint8_t *mbi)
{
    /* serial port output only */
    if (boot_get_frame_buffer(mbi) < 0) {
        internal_boot_cursor_back = boot_cursor_back_no_fb;
        internal_boot_back_space = boot_back_space_no_fb;
        internal_boot_putchar_tab = boot_putchar_tab_no_fb;
        internal_boot_put_new_line = boot_put_new_line_no_fb;
        internal_boot_putchar = boot_putchar_no_fb;
        max_ch_nr_x = DEFAULT_TTY_WIDTH;
        max_ch_nr_y = DEFAULT_TTY_HEIGHT;
    } else {
        /* init for psf font */
        boot_init_font();
    }

    /* init for serial port */
    if (boot_init_com() < 0) {
        /* nothing for us to output! */
        if (internal_boot_putchar == boot_putchar_no_fb) {
            return -1;
        }

        /* frame buffer output only */
        internal_boot_cursor_back = boot_cursor_back_no_com;
        internal_boot_back_space = boot_back_space_no_com;
        internal_boot_putchar_tab = boot_putchar_tab_no_com;
        internal_boot_put_new_line = boot_put_new_line_no_com;
        internal_boot_putchar = boot_putchar_no_com;
    }

    /* clear the screen */
    boot_clear_screen();

    if (internal_boot_putchar == boot_putchar_no_fb) {
        boot_puts("Warning: running under no-graphic mode.\n");
    } else if (internal_boot_putchar == boot_putchar_no_com) {
        boot_puts("Warning: No serial port for us to output.\n");
    }

    boot_puts("Welcome to ClosureOS!");
    boot_puts("");
    boot_puts("Version 0.0.1");
    boot_puts("Copyright(c) 2024 arttnba3 <arttnba@gmail.com>\n");

    return 0;
}
