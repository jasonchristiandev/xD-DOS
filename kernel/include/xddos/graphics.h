#ifndef __XDDOS_GRAPHICS_H
#define __XDDOS_GRAPHICS_H

#include "xddos/psf.h"
#include "xddos/requests.h"
#include <stdint.h>

extern requests_framebuffer_t *fb;
void graphics_psf_put_char(psf_data_t *font, char c, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg);
void graphics_psf_put_text(psf_data_t *font, const char *str, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg);
void graphics_clear(uint32_t col);
void graphics_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t col);

#endif // !__XDDOS_GRAPHICS_H
