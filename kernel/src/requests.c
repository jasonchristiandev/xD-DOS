#include "xddos/requests.h"
#include <stdbool.h>
#include <stddef.h>

boot_info_t boot_info;

requests_executable_file_t *request_executable_file() {
	static requests_executable_file_t res;
	res.address = boot_info.exefile->address;
	res.size = boot_info.exefile->size;
	return &res;
}

acpi_rsdp_t *request_rsdp() {
	// struct limine_rsdp_response *response = rsdp_request.response;
	// if (response == NULL || response->address == NULL) return NULL;
	// return (acpi_rsdp_t *) response->address;
	return NULL;
}

requests_framebuffers_t *request_framebuffers() {
	return boot_info.framebuffers;
}