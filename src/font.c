#include "xD-DOS/font.h"
#include "xD-DOS/logging.h"
#include <stdint.h>

void psf_init() {
	uint16_t glyph = 0;

	// Cast the address to PSF header struct
	psf_font *font = (psf_font *) &_binary_font_psf_start;

	if (font->magic != PSF_FONT_MAGIC) {
		LOG_ERROR("FONT", "Invalid font magic: got 0x%x, expected 0x%x", font->magic, PSF_FONT_MAGIC);
	} else {
		DEBUG_INFO("FONT", "Font valid. (width: %d, height: %d)", font->width, font->height);
	}

	// Exit if there is no unicode table
	if (font->flags == 0) {
		unicode = NULL;
		return;
	}

	int8_t *ptr = (int8_t *) ((uint8_t *) &_binary_font_psf_start + font->header_size + font->glyph_count * font->glyph_width);

	// Allocate memory for translation table
	unicode = calloc(USHRT_MAX, 2);

	while (ptr < (uint8_t *) &_binary_font_psf_end) {
		uint16_t uc = (uint16_t) ((uint8_t *) ptr[0]);
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
			} else {
				uc = 0;
			}
		}

		unicode[uc] = glyph;
		ptr++;
	}
}