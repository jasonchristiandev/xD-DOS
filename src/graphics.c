#include "xD-DOS/graphics.h"
#include "xD-DOS/font.h"
#include "xD-DOS/logging.h"
#include "xD-DOS/memory.h" // IWYU pragma: keep
#include "xD-DOS/requests.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

extern font_psf1_header_t *psf1_hdr;
extern font_psf2_data_t *psf2_hdr;
extern uint8_t *font_data_ptr;
extern int font_version;

void __put_char(xD_DOS_framebuffer_t *fb, uint32_t width, uint32_t height, uint32_t bytes_per_glyph, uint8_t *glyph_bitmap, uint32_t cx, uint32_t cy, uint32_t fg, uint32_t bg) {
	if (cy >= fb->height || cx >= fb->width) return;

	uint32_t *fb_ptr = (uint32_t *) fb->address;
	uint32_t bytes_per_row = (width + 7) / 8;

	for (uint32_t y = 0; y < height; y++) {
		uint8_t *row_data = &glyph_bitmap[y * bytes_per_row];
		uint32_t row_offset = (y + cy) * (fb->pitch / 4);

		for (uint32_t x = 0; x < width; x++) {
			if ((y + cy) >= fb->height || (x + cx) >= (fb->width)) continue;

			uint8_t byte = row_data[x / 8];
			uint8_t bit_mask = 1 << (7 - (x % 8));

			if (byte & bit_mask) {
				fb_ptr[row_offset + x + cx] = fg;
			} else {
				fb_ptr[row_offset + x + cx] = bg;
			}
		}
	}
}

void graphics_put_char(xD_DOS_framebuffer_t *fb, char ch, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg) {
	if (!fb || !fb->address) {
		LOG_ERROR("TERMGRAPHICS", "Framebuffer is NULL!");
		return;
	}

	if (unicode) ch = unicode[ch];

	uint32_t height;
	uint32_t width;
	uint32_t bytes_per_glyph;
	uint8_t *glyph_bitmap;

	if (font_version == 0) {
		LOG_ERROR("TERMGRAPHICS", "Font version equals 0!");
		return;
	} else if (font_version == 1) {
		height = psf1_hdr->char_size;
		width = 8; // PSF1 is always 8 pixels wide
		bytes_per_glyph = height;
	} else {
		height = psf2_hdr->height;
		width = psf2_hdr->width;
		bytes_per_glyph = psf2_hdr->bytes_per_glyph;
	}
	glyph_bitmap = font_data_ptr + (ch * bytes_per_glyph);

	__put_char(fb, width, height, bytes_per_glyph, glyph_bitmap, x, y, fg, bg);
}

void graphics_put_text(xD_DOS_framebuffer_t *fb, const char *str, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg) {
	if (!fb || !fb->address) {
		LOG_ERROR("TERMGRAPHICS", "Framebuffer is NULL!");
		return;
	}

	uint32_t height;
	uint32_t width;
	uint32_t bytes_per_glyph;

	if (font_version == 0) {
		LOG_ERROR("TERMGRAPHICS", "Font version equals 0!");
		return;
	} else if (font_version == 1) {
		height = psf1_hdr->char_size;
		width = 8; // PSF1 is always 8 pixels wide
		bytes_per_glyph = height;
	} else {
		height = psf2_hdr->height;
		width = psf2_hdr->width;
		bytes_per_glyph = psf2_hdr->bytes_per_glyph;
	}

	int term_x = x;
	int term_y = y;
	for (int i = 0; i < strlen(str); i++) {
		char ch = str[i];
		if (unicode) ch = unicode[ch];
		uint8_t *glyph_bitmap = font_data_ptr + (ch * bytes_per_glyph);
		if (ch == '\r') {
			term_x = x;
		} else if (ch == '\n') {
			term_y += height;
		} else if (ch == '\t') {
			ch = ' ';
			if (unicode) ch = unicode[' '];
			uint8_t *glyph_bitmap = font_data_ptr + (ch * bytes_per_glyph);
			for (int j = 0; j < 8; j++) {
				__put_char(fb, width, height, bytes_per_glyph, glyph_bitmap, term_x, term_y, fg, bg);
				term_x += width;
			}
		} else {
			__put_char(fb, width, height, bytes_per_glyph, glyph_bitmap, term_x, term_y, fg, bg);
			term_x += width;
		}
	}
}

void graphics_clear(xD_DOS_framebuffer_t *fb, uint32_t col) {
	uint32_t *fb_ptr = (uint32_t *) fb->address;
	uint32_t pixels = (fb->height * fb->pitch) / sizeof(uint32_t);
	for (int i = 0; i < pixels; i++) {
		fb_ptr[i] = col;
	}
}
