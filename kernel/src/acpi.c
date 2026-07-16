#include "xddos/acpi.h"
#include "xddos/logging.h"
#include "xddos/main.h"
#include "xddos/requests.h"
#include <string.h>

bool acpi_init() {
	acpi_rsdp_t *rsdp = request_rsdp();
	if (rsdp == NULL) return ACPI_INIT_NULL_RESPONSE;

	if (memcmp(rsdp->signature, "RSD PTR ", 8) != 0) return ACPI_INIT_CHECKSUM_FAIL;
	acpi_xsdt_t *xsdt = (acpi_xsdt_t *) (rsdp->xsdt_ptr + hhdm_offset);
	if (xsdt == NULL || memcmp(xsdt->header.signature, "XSDT", 4) != 0) {
		return ACPI_INIT_XSDT_NOT_FOUND;
	}
	LOG_DEBUG("ACPI", "Found XSDT: 0x%llx", xsdt);

	LOG_DEBUG("ACPI", "Done init.");

	return ACPI_INIT_OK;
}