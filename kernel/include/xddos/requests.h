#ifndef __XDDOS_REQUESTS_H
#define __XDDOS_REQUESTS_H

#include "xddos/acpi.h"
#include "xddos/pmm.h"
#include "xddos_boot/protocol.h"
#include <stdint.h>

extern boot_info_t boot_info;

typedef enum : uint8_t {
	MEMMAP_USABLE = 0,
	MEMMAP_RESERVED = 1,
	MEMMAP_ACPI_RECLAIMABLE = 2,
	MEMMAP_ACPI_NVS = 3,
	MEMMAP_BAD_MEMORY = 4,
	MEMMAP_BOOTLOADER_RECLAIMABLE = 5,
	MEMMAP_EXECUTABLE_AND_MODULES = 6,
	MEMMAP_FRAMEBUFFER = 7,
	MEMMAP_RESERVED_MAPPED = 8
} requests_memmap_flag_t;

typedef struct {
	void *address;
	uint64_t size;
} requests_executable_file_t;
typedef boot_framebuffer_t requests_framebuffer_t;
typedef boot_framebuffers_t requests_framebuffers_t;
requests_executable_file_t *request_executable_file();
acpi_rsdp_t *request_rsdp();
pmm_memmap_t *request_memmap();
requests_framebuffers_t *request_framebuffers();

#endif // !__XDDOS_REQUESTS_H