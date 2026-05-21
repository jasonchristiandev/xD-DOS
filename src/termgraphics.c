#include "xD-DOS/font.h"
#include "xD-DOS/logging.h"
#include "xD-DOS/requests.h"
#include <stddef.h>
#include <stdint.h>

extern psf1_header *psf1_hdr;
extern psf_font *psf2_hdr;
extern uint8_t *font_data_ptr;
extern int font_version;

void putchar(xD_DOS_framebuffer *fb, uint16_t c, int cx, int cy, uint32_t fg, uint32_t bg) {
	if (!fb || !fb->address) {
		LOG_ERROR("TERMGRAPHICS", "Framebuffer is NULL!");
		return;
	}

	if (unicode && c < USHRT_MAX) c = unicode[c];

	uint32_t height;
	uint32_t width;
	uint32_t glyph_width;
	uint8_t *glyph_bitmap;

	if (font_version == 0) {
		LOG_ERROR("TERMGRAPHICS", "Font version equals 0!");
		return;
	} else if (font_version == 1) {
		height = psf1_hdr->char_size;
		width = 8; // PSF1 is always 8 pixels wide
		glyph_width = height;
		glyph_bitmap = font_data_ptr + (c * glyph_width);
	} else {
		height = psf2_hdr->height;
		width = psf2_hdr->bytes_per_glyph;
		glyph_width = psf2_hdr->width;
		glyph_bitmap = font_data_ptr + (c * glyph_width);
	}

	uint32_t *fb_ptr = (uint32_t *) fb->address;
	fb_ptr[0] = 0xFFFFFFFF;

	if (cy >= fb->height || cx >= fb->width) return;

	for (uint32_t y = 0; y < height; y++) {
		uint32_t row_offset = (cy + y) * (fb->pitch / 4);
		uint8_t line = glyph_bitmap[y];

		for (uint32_t x = 0; x < width; x++) {
			if ((cy + y) >= fb->height || (cx + x) >= (fb->pitch / 4)) continue;

			// Check if the bit is set in the current line
			if (line & (1 << (width - 1 - x))) {
				fb_ptr[row_offset + cx + x] = fg;
			} else {
				fb_ptr[row_offset + cx + x] = bg;
			}
		}
	}
}