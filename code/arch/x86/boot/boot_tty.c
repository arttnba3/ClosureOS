#include <closureos/types.h>
#include <boot/multiboot2.h>
#include <graphics/font/psf.h>

extern char _binary_boot_font_psf_start[];
extern char _binary_boot_font_psf_end[];

/* frame buffer info */
static uint32_t *framebuffer_base; /* 32 bit color */
static uint32_t framebuffer_width;
static uint32_t framebuffer_height;

/* char output info */
static uint32_t cursor_x, cursor_y;    /* count by chars */
static uint32_t max_ch_nr_x, max_ch_nr_y;

/* font info */
static struct psf2_header *boot_font;
static uint32_t font_width, font_height;
static uint32_t font_width_bytes;
static uint8_t *glyph_table;
static uint32_t bytes_per_glyph, glyph_nr;

int boot_putchar_raw(uint16_t ch, uint32_t fg, uint32_t bg)
{
    uint8_t *glyph = glyph_table;
    size_t loc;

    if (ch == '\n') {
        goto new_line;
    }

    /* if char out of range, output null */
    if (ch < glyph_nr) {
        glyph += ch * bytes_per_glyph;
    }

    loc =  cursor_y * font_height * framebuffer_width;
    loc += cursor_x * font_width;

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
    cursor_x++;

    /* we may need to move to new line */
    if (cursor_x >= max_ch_nr_x) {
    new_line:
        cursor_x = 0;
        cursor_y++;
        
        /* we may need to scroll up */
        if (cursor_y >= max_ch_nr_y) {
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

            cursor_y--;
        }
    }
}

void boot_putchar(uint16_t ch)
{
    int left_ch_nr, space_nr;
    switch (ch) {
    case '\r':
        cursor_x = 0;
        break;

    case '\b':
        if (cursor_x > 0) {
            cursor_x--;
        }
        break;

    case '\t':
        left_ch_nr = max_ch_nr_x - cursor_x - 1;
        space_nr = left_ch_nr > 4 ? 4 : left_ch_nr;
        for (int i = 0; i < space_nr; i++) {
            boot_putchar(' ');
        }
        break;

    default:
        boot_putchar_raw(ch, 0xffffff, 0);
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

void boot_clear_screen(void)
{
    for (uint32_t y = 0; y < framebuffer_height; y++) {
        for (uint32_t x = 0; x < framebuffer_width; x++) {
            framebuffer_base[y * framebuffer_width + x] = 0;
        }
    }
}

int boot_get_frame_buffer(multiboot_uint8_t *mbi)
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

void boot_init_font(void)
{
    boot_font = (struct psf2_header*) _binary_boot_font_psf_start;

    font_width_bytes = (boot_font->width + 7) / 8;
    font_width = font_width_bytes * 8;
    font_height = boot_font->height;

    glyph_table = (uint8_t*)_binary_boot_font_psf_start+boot_font->header_size;
    glyph_nr = boot_font->glyph_nr;
    bytes_per_glyph = boot_font->bytes_per_glyph;

    cursor_x = cursor_y = 0;
    max_ch_nr_x = framebuffer_width / font_width;
    max_ch_nr_y = framebuffer_height / font_height;
}

int boot_tty_init(multiboot_uint8_t *mbi)
{
    if (boot_get_frame_buffer(mbi) < 0) {
        return -1;
    }

    /* clear the screen */
    boot_clear_screen();

    /* init for psf font */
    boot_init_font();

    boot_puts("Welcome to ClosureOS!");
    boot_puts("");
    boot_puts("Version 0.0.1");
    boot_puts("Copyright(c) 2023 arttnba3");

    return 0;
}
