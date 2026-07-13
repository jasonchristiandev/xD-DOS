#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "xddos/psf.h"
#include "xddos/requests.h"
#include <stdint.h>

void graphics_psf_put_char(requests_framebuffer_t *fb, psf_data_t *font, char c, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg);
void graphics_psf_put_text(requests_framebuffer_t *fb, psf_data_t *font, const char *str, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg);
void graphics_clear(requests_framebuffer_t *fb, uint32_t col);
void graphics_rect(requests_framebuffer_t *fb, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t col);

#endif // !GRAPHICS_H
