#ifndef FONT_H
#define FONT_H

#include <stdint.h>

#define PSF1_MAGIC 0x0436
#define PSF2_MAGIC 0x864ab572

// PSF1 glyph always have a width of 8 bits
// and height of char_size
typedef struct {
	uint16_t magic;	   // Magic bytes
	uint8_t font_mode; // Font mode
	uint8_t char_size; // Character size
} psf1_header_t;

typedef struct {
	uint32_t magic;			  // Magic bytes to identify PSF
	uint32_t version;		  //
	uint32_t header_size;	  // Offset of bitmaps in file
	uint32_t flags;			  // 0 if there's no unicode table
	uint32_t length;		  // Number of glyphs
	uint32_t bytes_per_glyph; // Size of each glyph
	uint32_t height;		  //
	uint32_t width;			  //
} psf2_header_t;

extern uint8_t _binary_font_psf_start[];
extern uint8_t _binary_font_psf_end[];

typedef struct {
	uint32_t version;
	psf1_header_t *psf1_header;
	psf2_header_t *psf2_header;
	uint16_t *unicode;
	uint8_t *data;
} psf_data_t;

psf_data_t *psf_init();

#endif // !FONT_H