#include "xddos/graphics.h"
#include "xddos/logging.h"
#include "xddos/psf.h"
#include "xddos/requests.h"
#include "xddos/stdlib.h" // IWYU pragma: keep
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static void put_char(xddos_framebuffer_t *fb, uint32_t width, uint32_t height, uint8_t *glyph_bitmap, uint32_t cx, uint32_t cy, uint32_t fg, uint32_t bg) {
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

void xddos_graphics_put_char(xddos_framebuffer_t *fb, xddos_psf_data_t *font, char ch, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg) {
	if (!fb || !fb->address) {
		LOG_ERROR("GRAPHICS", "Framebuffer is NULL!");
		return;
	}

	if (font->unicode) ch = font->unicode[(uint8_t) ch];

	uint32_t height;
	uint32_t width;
	uint32_t bytes_per_glyph;
	uint8_t *glyph_bitmap;

	if (font->version == 0) {
		LOG_ERROR("GRAPHICS", "Font version equals 0!");
		return;
	} else if (font->version == 1) {
		height = font->psf1_header->char_size;
		width = 8; // PSF1 is always 8 pixels wide
		bytes_per_glyph = height;
	} else {
		height = font->psf2_header->height;
		width = font->psf2_header->width;
		bytes_per_glyph = font->psf2_header->bytes_per_glyph;
	}
	glyph_bitmap = font->data + (ch * bytes_per_glyph);

	put_char(fb, width, height, glyph_bitmap, x, y, fg, bg);
}

void xddos_graphics_put_text(xddos_framebuffer_t *fb, xddos_psf_data_t *font, const char *str, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg) {
	if (!fb || !fb->address) {
		LOG_ERROR("GRAPHICS", "Framebuffer is NULL!");
		return;
	}

	uint32_t height;
	uint32_t width;
	uint32_t bytes_per_glyph;

	if (font->version == 0) {
		LOG_ERROR("GRAPHICS", "Font version equals 0!");
		return;
	} else if (font->version == 1) {
		height = font->psf1_header->char_size;
		width = 8; // PSF1 is always 8 pixels wide
		bytes_per_glyph = height;
	} else {
		height = font->psf2_header->height;
		width = font->psf2_header->width;
		bytes_per_glyph = font->psf2_header->bytes_per_glyph;
	}

	uint32_t term_x = x;
	uint32_t term_y = y;
	for (size_t i = 0; i < strlen(str); i++) {
		unsigned char ch = str[i];
		if (font->unicode) ch = font->unicode[ch];
		uint8_t *glyph_bitmap = font->data + (ch * bytes_per_glyph);
		if (ch == '\r') {
			term_x = x;
		} else if (ch == '\n') {
			term_y += height;
		} else if (ch == '\t') {
			ch = ' ';
			if (font->unicode) ch = font->unicode[' '];
			uint8_t *glyph_bitmap = font->data + (ch * bytes_per_glyph);
			for (uint8_t j = 0; j < 8; j++) {
				put_char(fb, width, height, glyph_bitmap, term_x, term_y, fg, bg);
				term_x += width;
			}
		} else {
			put_char(fb, width, height, glyph_bitmap, term_x, term_y, fg, bg);
			term_x += width;
		}
	}
}

void xddos_graphics_clear(xddos_framebuffer_t *fb, uint32_t col) {
	uint32_t *fb_ptr = (uint32_t *) fb->address;
	uint32_t pixels = (fb->height * fb->pitch) / sizeof(uint32_t);
	for (uint32_t i = 0; i < pixels; i++) {
		fb_ptr[i] = col;
	}
}
