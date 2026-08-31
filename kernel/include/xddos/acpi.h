#ifndef __XDDOS_ACPI_H
#define __XDDOS_ACPI_H

#include <stdbool.h>

typedef enum : uint8_t {
	ACPI_INIT_OK = 0,
	ACPI_INIT_NULL_RESPONSE = 1,
	ACPI_INIT_RSDP_SIGNATURE_FAIL = 2,
	ACPI_INIT_VERSION_NOT_SUPPORTED = 3,
	ACPI_INIT_XSDT_NOT_FOUND = 4,
	ACPI_INIT_MADT_NOT_FOUND = 5,
	ACPI_INIT_IO_APIC_ENTRY_NOT_FOUND = 6
} acpi_init_result_t;

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
} __attribute__((packed)) acpi_rsdp_t;

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
} __attribute__((packed)) acpi_descheader_t;

typedef struct {
	acpi_descheader_t header; // RSDT
	uint32_t entries[];
} __attribute__((packed)) acpi_rsdt_t;

typedef struct {
	acpi_descheader_t header; // XSDT
	uint64_t entries[];
} __attribute__((packed)) acpi_xsdt_t;

typedef struct {
	acpi_descheader_t header; // APIC
	uint32_t lic_ptr;
	uint32_t flags;
} __attribute__((packed)) acpi_madt_t;

typedef struct {
	uint8_t type;
	uint8_t length;
} __attribute__((packed)) acpi_madt_entry_t;

typedef struct {
	uint8_t type;
	uint8_t length;
	uint8_t io_apic_id;
	uint8_t reserved;
	uint32_t io_apic_ptr;
	uint32_t gsi_base;
} __attribute__((packed)) acpi_io_apic_entry_t;

typedef struct {
	uint8_t type;
	uint8_t length;
	uint8_t bus;
	uint8_t source;
	uint32_t gsi;
	uint16_t flags;
} __attribute__((packed)) acpi_iso_entry_t;

typedef enum : uint8_t {
	ACPI_PMPROFILE_UNSPECIFIED = 0,
	ACPI_PMPROFILE_DESKTOP = 1,
	ACPI_PMPROFILE_MOBILE = 2,
	ACPI_PMPROFILE_WORKSTATION = 3,
	ACPI_PMPROFILE_ENTERPRISESERVER = 4,
	ACPI_PMPROFILE_SOHOSERVER = 5,
	ACPI_PMPROFILE_APPLIANCEPC = 6,
	ACPI_PMPROFILE_PERFORMANCESERVER = 7,
	ACPI_PMPROFILE_TABLET = 8,
	ACPI_PMPROFILE_RESERVED = 9
} acpi_pmprofile_t;

typedef struct {
	acpi_descheader_t header; // FADT
	uint32_t firmware_ctrl;
	uint32_t dsdt_ptr;
	uint8_t reserved_a;
	acpi_pmprofile_t preferred_pm_profile;

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
} __attribute__((packed)) acpi_fadt_t;

extern acpi_rsdp_t *acpi_rsdp;
extern acpi_xsdt_t *acpi_xsdt;
extern acpi_madt_t *acpi_madt;
extern acpi_io_apic_entry_t *acpi_io_apic_entry;
extern uint32_t irq_to_gsi[16];

acpi_init_result_t acpi_init();
acpi_io_apic_entry_t *acpi_search_io_apic_entry();

#endif // !__XDDOS_ACPI_H