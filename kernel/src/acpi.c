#include "xddos/acpi.h"
#include "xddos/requests.h"
#include <string.h>

bool xddos_acpi_init() {
	xddos_acpi_rsdp_t *rsdp = xddos_request_rsdp();

	if (memcmp(rsdp->signature, "RSD PTR ", 8) != 0) return false;
	xddos_acpi_rsdt_t *rsdt = (xddos_acpi_rsdt_t *) rsdp->rsdt_ptr;
	xddos_acpi_xsdt_t *xsdt = (xddos_acpi_xsdt_t *) rsdp->xsdt_ptr;
	if (memcmp(rsdt->header.signature, "RSDT", 4) == 0) {
		
	} else if (memcmp(xsdt->header.signature, "XSDT", 4)) {

	} else {
		return false;
	}
}