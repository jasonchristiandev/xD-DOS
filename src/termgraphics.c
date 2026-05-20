#include "xD-DOS/font.h"
#include "xD-DOS/requests.h"
#include <stddef.h>
#include <stdint.h>

extern uint16_t *unicode;
extern uint8_t _binary_font_psf_start[];
extern uint8_t _binary_font_psf_end[];

void putchar(xD_DOS_framebuffer *fb, uint16_t c, int cx, int cy, uint32_t fg, uint32_t bg) {
	uint32_t scanline = fb->pitch;

	psf_font *font = (psf_font *) _binary_font_psf_start;

	if (unicode != NULL) {
		c = unicode[c];
	}

	if (c >= font->glyph_count) {
		c = 0;
	}

	uint32_t glyphline_width = (font->width + 7) / 8;
	uint32_t glyph_bytes = font->height * glyphline_width;

	uint8_t *glyph = _binary_font_psf_start + font->header_size + (c * glyph_bytes);

	uint32_t offs = (cy * font->height * scanline) + (cx * font->width * 4);

	uint8_t *fb_base = (uint8_t *) fb->address;

	for (uint32_t y = 0; y < font->height; y++) {
		uint32_t *line_ptr = (uint32_t *) (fb_base + offs);
		uint8_t *cur = glyph + (glyphline_width * y);
		uint8_t mask = 1 << 7;

		for (uint32_t x = 0; x < font->width; x++) {
			line_ptr[x] = (*cur & mask) ? fg : bg;

			mask >>= 1;
			if (mask == 0) {
				mask = 1 << 7;
				cur++;
			}
		}

		offs += scanline;
	}
}