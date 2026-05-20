#ifndef REQUESTS_H
#define REQUESTS_H

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
} xD_DOS_executable_address;
typedef struct {
	void *address;
	uint64_t size;
} xD_DOS_executable_file;
typedef struct limine_memmap_entry xD_DOS_memmap_entry;
typedef struct {
	uint64_t count;
	xD_DOS_memmap_entry **entries;
} xD_DOS_memmap;
typedef struct {
	uint64_t offset;
} xD_DOS_hhdm;
typedef struct limine_framebuffer xD_DOS_framebuffer;
typedef struct {
	uint64_t count;
	xD_DOS_framebuffer **framebuffers;
} xD_DOS_framebuffers;

uint8_t request_base_revision_supported();
xD_DOS_executable_address *request_executable_address();
xD_DOS_executable_file *request_executable_file();
xD_DOS_memmap *request_memmap();
xD_DOS_hhdm *request_hhdm();
xD_DOS_framebuffers *request_framebuffers();

#endif // !REQUESTS_H