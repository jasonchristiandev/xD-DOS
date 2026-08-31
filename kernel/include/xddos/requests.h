#ifndef __XDDOS_REQUESTS_H
#define __XDDOS_REQUESTS_H

#include "xddos/acpi.h"
#include <stdint.h>

#define MAX_MEMMAP_ENTRIES 256

// memmap
typedef enum : uint64_t {
	REQUESTS_MEMMAP_USABLE = 0,
	REQUESTS_MEMMAP_RESERVED = 1,
	REQUESTS_MEMMAP_ACPI_RECLAIMABLE = 2,
	REQUESTS_MEMMAP_ACPI_NVS = 3,
	REQUESTS_MEMMAP_BAD_MEMORY = 4,
	REQUESTS_MEMMAP_BOOTLOADER_RECLAIMABLE = 5,
	REQUESTS_MEMMAP_EXECUTABLE_AND_MODULES = 6,
	REQUESTS_MEMMAP_FRAMEBUFFER = 7,
	REQUESTS_MEMMAP_RESERVED_MAPPED = 8
} requests_memmap_flag_t;

typedef struct {
	uint64_t base;
	uint64_t length;
	requests_memmap_flag_t type;
} requests_memmap_entry_t;

typedef struct {
	uint64_t count;
	requests_memmap_entry_t entries[MAX_MEMMAP_ENTRIES];
} requests_memmap_t;

// framebuffer
typedef struct {
	uint64_t width;
	uint64_t height;
	uint64_t pitch;
	uint16_t bpp;
	uint8_t red_mask_size;
	uint8_t red_mask_shift;
	uint8_t green_mask_size;
	uint8_t green_mask_shift;
	uint8_t blue_mask_size;
	uint8_t blue_mask_shift;
} requests_video_mode_t;

typedef struct {
	uint64_t count;
	requests_video_mode_t **entries;
} requests_video_modes_t;

typedef struct {
	void *address;
	uint64_t size;
	uint64_t width;
	uint64_t height;
	uint64_t pitch;
	uint16_t bpp;
	requests_video_modes_t modes;
} requests_framebuffer_t;

typedef struct {
	uint64_t count;
	requests_framebuffer_t **entries;
} requests_framebuffers_t;

// boot info
typedef struct {
	requests_memmap_t *memmap;
	requests_framebuffers_t *framebuffers;
	acpi_rsdp_t *rsdp;
} boot_info_t;

extern boot_info_t boot_info;

#endif // !__XDDOS_REQUESTS_H