#include "xD-DOS/requests.h"
#include <limine.h>
#include <stddef.h>
#include <stdint.h>

extern volatile uint64_t limine_base_revision[];
extern volatile uint64_t limine_requests_start_marker[];
extern volatile uint64_t limine_requests_end_marker[];
extern volatile struct limine_executable_address_request executable_address_request;
extern volatile struct limine_executable_file_request executable_file_request;
extern volatile struct limine_memmap_request memmap_request;
extern volatile struct limine_hhdm_request hhdm_request;
extern volatile struct limine_framebuffer_request framebuffer_request;

uint8_t request_base_revision_supported() {
	return LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision);
}

xD_DOS_executable_address *request_executable_address() {
	struct limine_executable_address_response *response = executable_address_request.response;
	if (response == NULL) return NULL;
	static xD_DOS_executable_address res;
	res.phys = response->physical_base;
	res.virt = response->virtual_base;
	return &res;
}

xD_DOS_executable_file *request_executable_file() {
	struct limine_executable_file_response *response = executable_file_request.response;
	if (response == NULL) return NULL;
	static xD_DOS_executable_file res;
	res.address = response->executable_file->address;
	res.size = response->executable_file->size;
	return &res;
}

xD_DOS_memmap *request_memmap() {
	struct limine_memmap_response *response = memmap_request.response;
	if (response == NULL) return NULL;
	static xD_DOS_memmap res;
	res.count = response->entry_count;
	res.entries = response->entries;
	return &res;
}

xD_DOS_hhdm *request_hhdm() {
	struct limine_hhdm_response *response = hhdm_request.response;
	if (response == NULL) return NULL;
	static xD_DOS_hhdm res;
	res.offset = response->offset;
	return &res;
}

xD_DOS_framebuffers *request_framebuffers() {
	struct limine_framebuffer_response *response = framebuffer_request.response;
	if (response == NULL) return NULL;
	static xD_DOS_framebuffers res;
	res.count = response->framebuffer_count;
	res.framebuffers = response->framebuffers;
	return &res;
}