#ifndef FONT_H
#define FONT_H

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#define PSF1_FONT_MAGIC 0x0436

// PSF1 glyph always have a width of 8 bits
// and height of char_size
typedef struct {
	uint16_t magic;	   // Magic bytes
	uint8_t font_mode; // Font mode
	uint8_t char_size; // Character size
} PSF1_Header;

#define PSF_FONT_MAGIC 0x864ab572
typedef struct {
	uint32_t magic;			  // Magic bytes to identify PSF
	uint32_t version;		  //
	uint32_t header_size;	  // Offset of bitmaps in file
	uint32_t flags;			  // 0 if there's no unicode table
	uint32_t glyph_count;	  // Number of glyphs
	uint32_t bytes_per_glyph; // Size of each glyph
	uint32_t height;		  //
	uint32_t width;			  //
} PSF_font;

extern char _binary_font_psf_start;
extern char _binary_font_psf_end;

uint16_t *unicode;

void psf_init() {
	uint16_t glyph = 0;
	
	// Cast the address to PSF header struct
	PSF_font *font = (PSF_font *) &_binary_font_psf_start;
	
	// Exit if there is no unicode table
	if (font->flags == 0) {
		unicode = NULL;
		return;
	}
	
	char *ptr = (char *) ((unsigned char *) &_binary_font_psf_start +
	font->header_size +
	font->glyph_count * font->bytes_per_glyph);
	
	// Allocate memory for translation table
	unicode = calloc(USHRT_MAX, 2);
	
	while (ptr < (unsigned char *) &_binary_font_psf_end) {
		uint16_t uc = (uint16_t) ((unsigned char *) ptr[0]);
		if (uc == 0xFF) {
			glyph++;
			ptr++;
			continue;
		} else if (uc & 128) {
			// UTF-8 to unicode
			if ((uc & 32) == 0) {
				uc = ((ptr[0] & 0x1F) << 6) + (ptr[1] & 0x3F);
				ptr++;
			} else if ((uc & 16) == 0) {
				uc = ((((ptr[0] & 0xF) << 6) + (ptr[1] & 0x3F)) << 6) + (ptr[2] & 0x3F);
				ptr += 2;
			} else if ((uc & 8) == 0) {
				uc = ((((((ptr[0] & 0x7) << 6) + (ptr[1] & 0x3F)) << 6) + (ptr[2] & 0x3F)) << 6) + (ptr[3] & 0x3F);
				ptr += 3;
			} else
			uc = 0;
		}
		
		unicode[uc] = glyph;
		ptr++;
	}
}

#endif // !FONT_H