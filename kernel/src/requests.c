#include "xddos/requests.h"
#include <stdbool.h>
#include <stddef.h>

boot_info_t *boot_info;

requests_executable_address_t *request_executable_address() {
	// struct limine_executable_address_response *response = executable_address_request.response;
	// if (response == NULL) return NULL;
	// static requests_executable_address_t res;
	// res.phys = response->physical_base;
	// res.virt = response->virtual_base;
	// return &res;
	return NULL;
}

requests_executable_file_t *request_executable_file() {
	// struct limine_executable_file_response *response = executable_file_request.response;
	// if (response == NULL) return NULL;
	// static requests_executable_file_t res;
	// res.address = response->executable_file->address;
	// res.size = response->executable_file->size;
	// return &res;
	return NULL;
}

acpi_rsdp_t *request_rsdp() {
	// struct limine_rsdp_response *response = rsdp_request.response;
	// if (response == NULL || response->address == NULL) return NULL;
	// return (acpi_rsdp_t *) response->address;
	return NULL;
}

pmm_memmap_t *request_memmap() {
	static pmm_memmap_t res;
	res.count = boot_info->memmap->count;
	res.entries = (pmm_memmap_entry_t **) boot_info->memmap->entries;
	return &res;
}

requests_framebuffers_t *request_framebuffers() {
	return boot_info->framebuffers;
}