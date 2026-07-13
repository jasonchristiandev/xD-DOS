#include "xddos/requests.h"
#include <limine.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

__attribute__((used, section(".limine_requests"))) static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);
__attribute__((used, section(".limine_requests_start"))) static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".limine_requests_end"))) static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;
__attribute__((used, section(".limine_requests"))) static volatile struct limine_executable_address_request executable_address_request = {
	.id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID,
	.revision = 0};
__attribute__((used, section(".limine_requests"))) static volatile struct limine_executable_file_request executable_file_request = {
	.id = LIMINE_EXECUTABLE_FILE_REQUEST_ID,
	.revision = 0};
__attribute__((used, section(".limine_requests"))) static volatile struct limine_rsdp_request rsdp_request = {
	.id = LIMINE_RSDP_REQUEST_ID,
	.revision = 0};
__attribute__((used, section(".limine_requests"))) static volatile struct limine_memmap_request memmap_request = {
	.id = LIMINE_MEMMAP_REQUEST_ID,
	.revision = 0};
__attribute__((used, section(".limine_requests"))) static volatile struct limine_hhdm_request hhdm_request = {
	.id = LIMINE_HHDM_REQUEST_ID,
	.revision = 0};
__attribute__((used, section(".limine_requests"))) static volatile struct limine_framebuffer_request framebuffer_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST_ID,
	.revision = 0};

bool request_base_revision_supported() {
	return LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision);
}

requests_executable_address_t *request_executable_address() {
	struct limine_executable_address_response *response = executable_address_request.response;
	if (response == NULL) return NULL;
	static requests_executable_address_t res;
	res.phys = response->physical_base;
	res.virt = response->virtual_base;
	return &res;
}

requests_executable_file_t *request_executable_file() {
	struct limine_executable_file_response *response = executable_file_request.response;
	if (response == NULL) return NULL;
	static requests_executable_file_t res;
	res.address = response->executable_file->address;
	res.size = response->executable_file->size;
	return &res;
}

acpi_rsdp_t *request_rsdp() {
	struct limine_rsdp_response *response = rsdp_request.response;
	if (response == NULL || response->address == NULL) return NULL;
	return (acpi_rsdp_t *) response->address;
}

pmm_memmap_t *request_memmap() {
	struct limine_memmap_response *response = memmap_request.response;
	if (response == NULL) return NULL;
	static pmm_memmap_t res;
	res.count = response->entry_count;
	res.entries = (pmm_memmap_entry_t**)response->entries;
	return &res;
}

requests_hhdm_t *request_hhdm() {
	struct limine_hhdm_response *response = hhdm_request.response;
	if (response == NULL) return NULL;
	static requests_hhdm_t res;
	res.offset = response->offset;
	return &res;
}

requests_framebuffers_t *request_framebuffers() {
	struct limine_framebuffer_response *response = framebuffer_request.response;
	if (response == NULL) return NULL;
	static requests_framebuffers_t res;
	res.count = response->framebuffer_count;
	res.framebuffers = response->framebuffers;
	return &res;
}