#ifndef TERMGRAPHICS_H
#define TERMGRAPHICS_H

#include "xD-DOS/requests.h"
#include <stdint.h>

void putchar(xD_DOS_framebuffer *fb, uint16_t c, int32_t cx, int32_t cy, uint32_t fg, uint32_t bg);

#endif // !TERMGRAPHICS_H
