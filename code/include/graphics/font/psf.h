#ifndef GRAPHICS_FONT_PSF_H
#define GRAPHICS_FONT_PSF_H

#include <closureos/types.h>

#define PSF1_FONT_MAGIC 0x0436

struct psf1_header {
    uint16_t magic;     /* magic number for identification */
    uint8_t font_mode;  /* PSF font mode */
    uint8_t char_size;  /* PSF char size */
};

#define PSF2_FONT_MAGIC 0x864ab572

struct psf2_header {
    uint32_t magic;             /* magic number for PSF */
    uint32_t version;           /* zero */
    uint32_t header_size;       /* offset of bitmaps in file, 32 */
    uint32_t flags;             /* 0 if there's no unicode table */
    uint32_t glyph_nr;          /* number of glyphs */
    uint32_t bytes_per_glyph;   /* size of each glyph */
    uint32_t height;            /* height in pixels */
    uint32_t width;             /* width in pixels */
};

#endif // GRAPHICS_FONT_PSF_H
