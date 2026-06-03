#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "xddos/psf.h"
#include "xddos/requests.h"
#include <stdint.h>

void xddos_graphics_psf_put_char(xddos_framebuffer_t *fb, xddos_psf_data_t *font, char c, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg);
void xddos_graphics_psf_put_text(xddos_framebuffer_t *fb, xddos_psf_data_t *font, const char *str, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg);
void xddos_graphics_clear(xddos_framebuffer_t *fb, uint32_t col);

#endif // !GRAPHICS_H
