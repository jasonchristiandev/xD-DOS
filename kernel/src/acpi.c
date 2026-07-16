#include "xddos/acpi.h"
#include "xddos/logging.h"
#include "xddos/main.h"
#include "xddos/requests.h"
#include <string.h>

acpi_rsdp_t *rsdp;
acpi_xsdt_t *xsdt;
acpi_madt_t *madt;

acpi_init_result_t acpi_init() {
	rsdp = request_rsdp();
	if (rsdp == NULL) return ACPI_INIT_NULL_RESPONSE;
	if (memcmp(rsdp->signature, "RSD PTR ", 8) != 0) return ACPI_INIT_RSDP_CHECKSUM_FAIL;
	if (rsdp->revision < 2) return ACPI_INIT_VERSION_NOT_SUPPORTED;

	// xsdt
	xsdt = (acpi_xsdt_t *) (rsdp->xsdt_ptr + hhdm_offset);
	if (xsdt == NULL || memcmp(xsdt->header.signature, "XSDT", 4) != 0) {
		return ACPI_INIT_XSDT_NOT_FOUND;
	}
	LOG_DEBUG("ACPI", "Found XSDT: 0x%llx (%u bytes)", xsdt, xsdt->header.length);
	int entries = (xsdt->header.length - sizeof(xsdt->header)) / 8;

	// madt
	madt = NULL;
	for (int i = 0; i < entries; i++) {
		acpi_descheader_t *h = (acpi_descheader_t *) (xsdt->entries[i] + hhdm_offset);
		if (!strncmp(h->signature, "APIC", 4)) madt = (acpi_madt_t *) h;
	}
	if (madt == NULL) return ACPI_INIT_MADT_NOT_FOUND;
	LOG_DEBUG("ACPI", "Found MADT: 0x%llx (%u bytes)", madt, madt->header.length);

	LOG_DEBUG("ACPI", "Done init.");
	return ACPI_INIT_OK;
}

acpi_io_apic_entry_t *acpi_search_io_apic_entry() {
	uint8_t *cur = (uint8_t *) &madt->feh;
	uint8_t *end = ((uint8_t *) madt) + madt->header.length;

	while (cur < end) {
		acpi_madt_entry_t *entry = (acpi_madt_entry_t *) cur;

		if (entry->length == 0) break;
		if (entry->type == 1) return (acpi_io_apic_entry_t *) cur;

		cur += entry->length;
	}

	return NULL;
}
