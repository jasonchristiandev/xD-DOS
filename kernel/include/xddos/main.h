#ifndef MAIN_H
#define MAIN_H

#include "xddos/psf.h"
#include "xddos/requests.h"

extern uint64_t hhdm_offset;
extern requests_framebuffer_t *fb;
extern psf_data_t *fallback_font;
extern int32_t mouse_x, mouse_y;
void kernel_main();

#endif // !MAIN_H