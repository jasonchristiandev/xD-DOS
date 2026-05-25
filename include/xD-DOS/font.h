#ifndef FONT_H
#define FONT_H

#include "xD-DOS/memalloc.h" // IWYU pragma: keep
#include <limits.h>
#include <stdint.h>

#define PSF1_FONT_MAGIC 0x0436
#define PSF_FONT_MAGIC 0x864ab572

// PSF1 glyph always have a width of 8 bits
// and height of char_size
typedef struct font_psf1_header {
	uint16_t magic;	   // Magic bytes
	uint8_t font_mode; // Font mode
	uint8_t char_size; // Character size
} font_psf1_header_t;

typedef struct font_psf2_data {
	uint32_t magic;			  // Magic bytes to identify PSF
	uint32_t version;		  //
	uint32_t header_size;	  // Offset of bitmaps in file
	uint32_t flags;			  // 0 if there's no unicode table
	uint32_t length;		  // Number of glyphs
	uint32_t bytes_per_glyph; // Size of each glyph
	uint32_t height;		  //
	uint32_t width;			  //
} font_psf2_data_t;

extern uint8_t _binary_font_psf_start[];
extern uint8_t _binary_font_psf_end[];

extern uint16_t *unicode;
extern font_psf1_header_t *psf1_hdr;
extern font_psf2_data_t *psf2_hdr;
extern uint8_t *font_data_ptr;
extern int font_version;

void psf_init();

#endif // !FONT_H