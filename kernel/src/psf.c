#include "xddos/psf.h"
#include "xddos/logging.h"
#include <limits.h>
#include <string.h>

psf_data_t *fallback_font;

static void psf1_init(psf_data_t *data, uint8_t *virt_start, uint8_t *virt_end) {
	psf1_header_t *header = (psf1_header_t *) virt_start;
	data->psf1_header = header;
	data->version = 1;
	data->data = virt_start + sizeof(psf1_header_t);

	if ((header->font_mode & 2) == 0 && (header->font_mode & 4) == 0) {
		data->unicode = NULL;
		return;
	}

	memset(data->unicode, 0, sizeof(*(data->unicode)));
	uint32_t glyph_count = (header->font_mode & 1) ? 512 : 256;
	uint8_t *ptr = data->data + (glyph_count * header->char_size);

	uint16_t cur = 0;
	while (ptr < virt_end && cur < glyph_count) {
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

		if (val < USHRT_MAX && val > 0) data->unicode[val] = cur;
	}
}

static void psf2_init(psf_data_t *data, psf2_header_t *header, uint8_t *virt_end) {
	data->psf2_header = header;
	data->version = 2;
	data->data = ((uint8_t *) header) + header->header_size;

	if (header->flags == 0) {
		data->unicode = NULL;
		return;
	}

	memset(data->unicode, 0, sizeof(*(data->unicode)));
	uint8_t *ptr = ((uint8_t *) header) + header->header_size + (header->length * header->bytes_per_glyph);

	uint16_t glyph = 0;
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
		}

		if (uc & 128) {
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

		if (uc > 0) data->unicode[uc] = glyph;
	}
}

psf_data_t *psf_init() {
	static psf_data_t data;
	memset(&data, 0, sizeof(psf_data_t));

	uint32_t magic = *(uint32_t *) _binary_font_psf_start;
	if ((magic & 0xFFFF) == PSF1_MAGIC) {
		psf1_init(&data, _binary_font_psf_start, _binary_font_psf_end);
	} else if (magic == PSF2_MAGIC) {
		psf2_init(&data, (psf2_header_t *) _binary_font_psf_start, _binary_font_psf_end);
	}

	LOG_DEBUG("PSF", "Done init.");

	return &data;
}