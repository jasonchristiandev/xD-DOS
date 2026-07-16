#include "xddos/acpi.h"
#include "xddos/logging.h"
#include "xddos/main.h"
#include "xddos/requests.h"
#include <string.h>

acpi_rsdp_t *acpi_rsdp;
acpi_xsdt_t *acpi_xsdt;
acpi_madt_t *acpi_madt;
acpi_io_apic_entry_t *acpi_io_apic_entry;
uint32_t irq_to_gsi[16];

acpi_init_result_t acpi_init() {
	acpi_rsdp = request_rsdp();
	if (acpi_rsdp == NULL) return ACPI_INIT_NULL_RESPONSE;
	if (memcmp(acpi_rsdp->signature, "RSD PTR ", 8) != 0) return ACPI_INIT_RSDP_CHECKSUM_FAIL;
	if (acpi_rsdp->revision < 2) return ACPI_INIT_VERSION_NOT_SUPPORTED;

	// xsdt
	acpi_xsdt = (acpi_xsdt_t *) (acpi_rsdp->xsdt_ptr + hhdm_offset);
	if (acpi_xsdt == NULL || memcmp(acpi_xsdt->header.signature, "XSDT", 4) != 0) {
		return ACPI_INIT_XSDT_NOT_FOUND;
	}
	LOG_DEBUG("ACPI", "Found XSDT: 0x%llx (%u bytes)", acpi_xsdt, acpi_xsdt->header.length);
	int entries = (acpi_xsdt->header.length - sizeof(acpi_xsdt->header)) / 8;

	// madt
	acpi_madt = NULL;
	for (int i = 0; i < entries; i++) {
		acpi_descheader_t *h = (acpi_descheader_t *) (acpi_xsdt->entries[i] + hhdm_offset);
		if (!strncmp(h->signature, "APIC", 4)) acpi_madt = (acpi_madt_t *) h;
	}
	if (acpi_madt == NULL) return ACPI_INIT_MADT_NOT_FOUND;
	LOG_DEBUG("ACPI", "Found MADT: 0x%llx (%u bytes)", acpi_madt, acpi_madt->header.length);

	uint8_t *cur = (uint8_t *) ((uint64_t) acpi_madt + sizeof(acpi_madt_t));
	uint8_t *end = (uint8_t *) ((uint64_t) acpi_madt + acpi_madt->header.length);

	for (int i = 0; i < 16; i++) {
		irq_to_gsi[i] = i;
	}

	while (cur < end) {
		acpi_madt_entry_t *entry = (acpi_madt_entry_t *) cur;

		if (entry->length == 0) break;
		if (entry->type == 1) {
			acpi_io_apic_entry = (acpi_io_apic_entry_t *) cur;
			LOG_DEBUG("ACPI", "Found I/O APIC: 0x%llx", acpi_io_apic_entry);
		}
		if (entry->type == 2) {
			acpi_iso_entry_t *iso = (acpi_iso_entry_t *) cur;
			LOG_DEBUG("ACPI", "Found ISO: 0x%llx", iso);

			if (iso->source < 16) {
				irq_to_gsi[iso->source] = iso->gsi;
			}
		}

		cur += entry->length;
	}

	if (acpi_io_apic_entry == NULL) return ACPI_INIT_IO_APIC_ENTRY_NOT_FOUND;

	LOG_DEBUG("ACPI", "Done init.");
	return ACPI_INIT_OK;
}

acpi_io_apic_entry_t *acpi_search_io_apic_entry() {

	return NULL;
}
