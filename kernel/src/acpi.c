#include "xddos/acpi.h"
#include "xddos/asm.h"
#include "xddos/pit.h"
#include "xddos/requests.h"
#include <stdint.h>
#include <string.h>

extern xddos_acpi_fadt_t *global_fadt;

void xddos_acpi_init(void) {
	xddos_rsdp_t *rsdp = xddos_request_rsdp();
	if (!rsdp) return;

	if (memcmp(rsdp->signature, "RSD PTR ", 8) != 0) return;

	xddos_acpi_header_t *fadt = NULL;

	if (rsdp->revision >= 2 && rsdp->xsdt_ptr != 0) {
		xddos_acpi_xsdt_t *xsdt = (xddos_acpi_xsdt_t *) rsdp->xsdt_ptr;

		int entries = (xsdt->header.length - sizeof(xddos_acpi_header_t)) / 8;

		for (int i = 0; i < entries; i++) {
			xddos_acpi_header_t *table = (xddos_acpi_header_t *) xsdt->tables[i];
			if (memcmp(table->signature, "FACP", 4) == 0) {
				fadt = table;
				break;
			}
		}
	} else if (rsdp->rsdt_ptr != 0) {
		xddos_acpi_rsdt_t *rsdt = (xddos_acpi_rsdt_t *) rsdp->rsdt_ptr;

		int entries = (rsdt->header.length - sizeof(xddos_acpi_header_t)) / 4;

		for (int i = 0; i < entries; i++) {
			xddos_acpi_header_t *table = (xddos_acpi_header_t *) ((uintptr_t) rsdt->tables[i]);
			if (memcmp(table->signature, "FACP", 4) == 0) {
				fadt = table;
				break;
			}
		}
	}

	if (fadt != NULL) {
		xddos_acpi_parse_fadt(fadt);
	}
}

void xddos_acpi_enable(xddos_acpi_fadt_t *fadt) {
	if ((inw(fadt->pm1a_control_block) & 1) == 0) {
		if (fadt->smi_cmd != 0) {
			outb(fadt->smi_cmd, fadt->acpi_enable);

			int timeout = 0;
			while ((inw(fadt->pm1a_control_block) & 1) == 0) {
				xddos_pit_sleep_ms(1);
				if (timeout++ > 5000) {
					return;
				}
			}
		}
	}
}


void xddos_acpi_parse_fadt(xddos_acpi_header_t *table) {
	global_fadt = (xddos_acpi_fadt_t *) table;
	xddos_acpi_enable(global_fadt);
}