#ifndef REQUESTS_H
#define REQUESTS_H

#include "xddos/acpi.h"
#include <limine.h>
#include <stdint.h>

#define XD_DOS_MEMMAP_USABLE 0
#define XD_DOS_MEMMAP_RESERVED 1
#define XD_DOS_MEMMAP_ACPI_RECLAIMABLE 2
#define XD_DOS_MEMMAP_ACPI_NVS 3
#define XD_DOS_MEMMAP_BAD_MEMORY 4
#define XD_DOS_MEMMAP_BOOTLOADER_RECLAIMABLE 5
#define XD_DOS_MEMMAP_EXECUTABLE_AND_MODULES 6
#define XD_DOS_MEMMAP_FRAMEBUFFER 7
#define XD_DOS_MEMMAP_RESERVED_MAPPED 8

typedef struct {
	uint64_t phys;
	uint64_t virt;
} xddos_executable_address_t;
typedef struct {
	void *address;
	uint64_t size;
} xddos_executable_file_t;
typedef struct limine_memmap_entry xddos_memmap_entry_t;
typedef struct {
	uint64_t count;
	xddos_memmap_entry_t **entries;
} xddos_memmap_t;
typedef struct {
	uint64_t offset;
} xddos_hhdm_t;
typedef struct limine_framebuffer xddos_framebuffer_t;
typedef struct {
	uint64_t count;
	xddos_framebuffer_t **framebuffers;
} xddos_framebuffers_t;

uint8_t xddos_request_base_revision_supported();
xddos_executable_address_t *xddos_request_executable_address();
xddos_executable_file_t *xddos_request_executable_file();
xddos_acpi_rsdp_t *xddos_request_rsdp();
xddos_memmap_t *xddos_request_memmap();
xddos_hhdm_t *xddos_request_hhdm();
xddos_framebuffers_t *xddos_request_framebuffers();

#endif // !REQUESTS_H