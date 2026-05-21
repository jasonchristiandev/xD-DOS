#ifndef TERMGRAPHICS_H
#define TERMGRAPHICS_H

#include "xD-DOS/requests.h"
#include <stdint.h>

void put_char(xD_DOS_framebuffer *fb, char c, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg);
void put_text(xD_DOS_framebuffer *fb, const char *str, uint32_t fg, uint32_t bg);

#endif // !TERMGRAPHICS_H
