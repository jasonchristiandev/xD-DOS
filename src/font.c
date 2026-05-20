#include "xD-DOS/font.h"
#include "xD-DOS/logging.h"
#include "xD-DOS/memalloc.h"
#include "xD-DOS/requests.h"
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

extern uint64_t hhdm_offset;

void psf1_init(uint8_t *virt_start, uint8_t *virt_end) {
	psf1_header *font = (psf1_header *) virt_start;

	if (font->magic != PSF1_FONT_MAGIC) {
		LOG_ERROR("FONT", "Invalid font magic: got 0x%x, expected 0x%x. Giving up...", font->magic, PSF1_FONT_MAGIC);
		return;
	}

	LOG_INFO("FONT", "Font valid. (height: %d)", font->char_size);

	if ((font->font_mode & 2) == 0 && (font->font_mode & 4) == 0) {
		unicode = NULL;
		return;
	}

	unicode = calloc(USHRT_MAX, 2);
	if (unicode == NULL) return;

	uint32_t glyph_count = (font->font_mode & 1) ? 512 : 256;

	uint8_t *ptr = virt_start + 4 + (glyph_count * font->char_size);
	uint8_t *end = virt_end;

	uint16_t cur = 0;

	while (ptr < end && cur < glyph_count) {
		uint8_t uc = ptr[0];

		if (uc == 0xFF) {
			cur++;
			ptr++;
			continue;
		}
		if (uc == 0xFE) {
			ptr++;
			continue;
		}

		uint16_t val = 0;
		if ((uc & 0x80) == 0) {
			val = uc;
			ptr += 1;
		} else if ((uc & 0xE0) == 0xC0) {
			val = ((ptr[0] & 0x1F) << 6) | (ptr[1] & 0x3F);
			ptr += 2;
		} else if ((uc & 0xF0) == 0xE0) {
			val = ((ptr[0] & 0x0F) << 12) | ((ptr[1] & 0x3F) << 6) | (ptr[2] & 0x3F);
			ptr += 3;
		} else {
			ptr++;
			continue;
		}

		if (val < USHRT_MAX && val > 0) {
			unicode[val] = cur;
		}
	}
}

void psf2_init(psf_font *font, uint8_t *virt_end) {
	uint16_t glyph = 0;

	if (font->flags == 0) {
		unicode = NULL;
		return;
	}

	uint8_t *ptr = (uint8_t *) font + font->header_size + font->glyph_count * font->glyph_width;

	unicode = calloc(USHRT_MAX, 2);
	if (unicode == NULL) return;

	while (ptr < virt_end) {
		uint8_t uc = ptr[0];
		if (uc == 0xFF) {
			glyph++;
			ptr++;
			continue;
		}
		if (uc == 0xFE) {
			ptr++;
			continue;
		} else if (uc & 128) {
			if ((uc & 32) == 0) {
				uc = ((ptr[0] & 0x1F) << 6) + (ptr[1] & 0x3F);
				ptr += 2;
			} else if ((uc & 16) == 0) {
				uc = ((((ptr[0] & 0xF) << 6) + (ptr[1] & 0x3F)) << 6) + (ptr[2] & 0x3F);
				ptr += 3;
			} else if ((uc & 8) == 0) {
				uc = ((((((ptr[0] & 0x7) << 6) + (ptr[1] & 0x3F)) << 6) + (ptr[2] & 0x3F)) << 6) + (ptr[3] & 0x3F);
				ptr += 4;
			} else {
				ptr++;
				continue;
			}
		} else {
			uc = ptr[0];
			ptr += 1;
		}

		if (uc > 0) {
			unicode[uc] = glyph;
		}
	}
}

void psf_init() {
	xD_DOS_executable_address *exeaddr = request_executable_address();
	if (!exeaddr) {
		LOG_ERROR("FONT", "Executable address is NULL!");
		return;
	}

	uint64_t font_virt = (uint64_t) &_binary_font_psf_start;

	uint64_t kernel_offset = font_virt - exeaddr->virt;
	uint64_t font_phys_addr = exeaddr->phys + kernel_offset;

	uint8_t *virt_start = (uint8_t *) (font_phys_addr + hhdm_offset);
	uint8_t *virt_end = virt_start + ((uint64_t) &_binary_font_psf_end - (uint64_t) &_binary_font_psf_start);

	psf_font *font = (psf_font *) virt_start;

	if (font->magic != PSF_FONT_MAGIC) {
		LOG_INFO("FONT", "Invalid font magic: got 0x%x, expected 0x%x. Trying PSF1...", font->magic, PSF_FONT_MAGIC);
		psf1_init(virt_start, virt_end);
	} else {
		DEBUG_INFO("FONT", "Font valid. (width: %d, height: %d)", font->width, font->height);
		psf2_init(font, virt_end);
	}
}