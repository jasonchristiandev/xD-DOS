#ifndef ACPI_H
#define ACPI_H

#include <stdbool.h>

// https://uefi.org/sites/default/files/resources/ACPI_6_3_final_Jan30.pdf page 118
typedef struct {
	char signature[8]; // must be exactly "RSD PTR "
	uint8_t checksum;
	char oem_id[6];
	uint8_t revision;
	uint32_t rsdt_ptr;
	uint32_t length;
	uint64_t xsdt_ptr;
	uint8_t ex_checksum;
	uint8_t reserved[3];
} __attribute__((packed)) xddos_acpi_rsdp_t;

typedef struct {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oem_id[6];
	char oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
} __attribute__((packed)) xddos_acpi_descheader_t;

typedef struct {
	xddos_acpi_descheader_t header; // RSDT
	uint32_t entries[];
} __attribute__((packed)) xddos_acpi_rsdt_t;

typedef struct {
	xddos_acpi_descheader_t header; // XSDT
	uint64_t entries[];
} __attribute__((packed)) xddos_acpi_xsdt_t;

typedef enum : uint8_t {
	XDDOS_ACPI_PMPROFILE_UNSPECIFIED = 0,
	XDDOS_ACPI_PMPROFILE_DESKTOP = 1,
	XDDOS_ACPI_PMPROFILE_MOBILE = 2,
	XDDOS_ACPI_PMPROFILE_WORKSTATION = 3,
	XDDOS_ACPI_PMPROFILE_ENTERPRISESERVER = 4,
	XDDOS_ACPI_PMPROFILE_SOHOSERVER = 5,
	XDDOS_ACPI_PMPROFILE_APPLIANCEPC = 6,
	XDDOS_ACPI_PMPROFILE_PERFORMANCESERVER = 7,
	XDDOS_ACPI_PMPROFILE_TABLET = 8,
	XDDOS_ACPI_PMPROFILE_RESERVED = 9
} xddos_acpi_pm_profile_t;

typedef struct {
	xddos_acpi_descheader_t header; // FADT
	uint32_t firmware_ctrl;
	uint32_t dsdt_ptr;
	uint8_t reserved_a;
	xddos_acpi_pm_profile_t preferred_pm_profile;

	uint16_t sci_interrupt;
	uint32_t smi_command_port;
	uint8_t acpi_enable;
	uint8_t acpi_disable;
	uint8_t s4bios_req;
	uint8_t pstate_control;

	uint32_t pm1a_event_block;
	uint32_t pm1b_event_block;
	uint32_t pm1a_control_block;
	uint32_t pm1b_control_block;
	uint32_t pm2_control_block;
	uint32_t pm_timer_block;

	// more fields later when the os need it
} __attribute__((packed)) xddos_acpi_fadt_t;

bool xddos_acpi_init();

#endif // !ACPI_H