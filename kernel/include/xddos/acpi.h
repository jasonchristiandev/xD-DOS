#ifndef ACPI_H
#define ACPI_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oem[6];
	char oem_table[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
} __attribute__((packed)) xddos_acpi_header_t;

typedef struct {
	xddos_acpi_header_t header;
	uint64_t tables[];
} __attribute__((packed)) xddos_acpi_xsdt_t;

typedef struct {
	xddos_acpi_header_t header;
	uint32_t tables[];
} __attribute__((packed)) xddos_acpi_rsdt_t;

#include <stdint.h>

typedef struct {
	xddos_acpi_header_t header;
	uint32_t facs_ptr;
	uint32_t dsdt_ptr;
	uint8_t reserved0;
	uint8_t preferred_pm_profile;
	uint16_t sci_int;

	uint32_t smi_cmd;
	uint8_t acpi_enable;
	uint8_t acpi_disable;
	uint8_t s4bios_req;
	uint8_t pstate_cnt;

	uint32_t pm1a_event_block;
	uint32_t pm1b_event_block;
	uint32_t pm1a_control_block;
	uint32_t pm1b_control_block;
} __attribute__((packed)) xddos_acpi_fadt_t;

static xddos_acpi_fadt_t *global_fadt = NULL;

void xddos_acpi_init();
void xddos_acpi_parse_fadt(xddos_acpi_header_t *table);

#endif // !ACPI_H