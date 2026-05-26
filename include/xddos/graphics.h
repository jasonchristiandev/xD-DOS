#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "xddos/font.h"
#include "xddos/requests.h"
#include <stdint.h>

void graphics_put_char(xddos_framebuffer_t *fb, font_data_t *font, char c, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg);
void graphics_put_text(xddos_framebuffer_t *fb, font_data_t *font, const char *str, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg);
void graphics_clear(xddos_framebuffer_t *fb, uint32_t col);

#endif // !GRAPHICS_H
