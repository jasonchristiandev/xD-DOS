#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "xD-DOS/requests.h"
#include <stdint.h>

void graphics_put_char(xD_DOS_framebuffer_t *fb, char c, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg);
void graphics_put_text(xD_DOS_framebuffer_t *fb, const char *str, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg);
void graphics_clear(xD_DOS_framebuffer_t *fb, uint32_t col);

#endif // !GRAPHICS_H
