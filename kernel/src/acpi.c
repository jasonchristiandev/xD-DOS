#include "xddos/acpi.h"
#include "xddos/requests.h"
#include <string.h>

bool acpi_init() {
	acpi_rsdp_t *rsdp = request_rsdp();

	if (memcmp(rsdp->signature, "RSD PTR ", 8) != 0) return false;
	acpi_rsdt_t *rsdt = (acpi_rsdt_t *) rsdp->rsdt_ptr;
	acpi_xsdt_t *xsdt = (acpi_xsdt_t *) rsdp->xsdt_ptr;
	if (memcmp(rsdt->header.signature, "RSDT", 4) == 0) {

	} else if (memcmp(xsdt->header.signature, "XSDT", 4)) {

	} else {
		return false;
	}

	return false;
}