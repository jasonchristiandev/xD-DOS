#include "xddos/main.h"
#include "xddos/gdt.h"
#include "xddos/graphics.h"
#include "xddos/interrupts.h"
#include "xddos/logging.h"
#include "xddos/pit.h"
#include "xddos/ps2.h"
#include "xddos/vma.h"

int32_t mouse_x = 0, mouse_y = 0;

void kernel_main() {
	LOG_DEBUG("VMM", "Done init.");

	// init acpi
	acpi_init_result_t acpi_result = acpi_init();
	char *acpi_result_name[7] = {
		[ACPI_INIT_OK] = "OK",
		[ACPI_INIT_NULL_RESPONSE] = "NULL_RESPONSE",
		[ACPI_INIT_RSDP_CHECKSUM_FAIL] = "CHECKSUM_FAIL",
		[ACPI_INIT_VERSION_NOT_SUPPORTED] = "VERSION_NOT_SUPPORTED",
		[ACPI_INIT_XSDT_NOT_FOUND] = "XSDT_NOT_FOUND",
		[ACPI_INIT_MADT_NOT_FOUND] = "MADT_NOT_FOUND",
		[ACPI_INIT_IO_APIC_ENTRY_NOT_FOUND] = "IO_APIC_ENTRY_NOT_FOUND"};
	if (acpi_result != 0) {
		char *name = "UNKNOWN";
		if (acpi_result < 7) name = acpi_result_name[acpi_result];
		interrupts_fail("ACPI_INIT bad return!", acpi_result, name);
	}

	// init gdt
	LOG_DEBUG("KERNEL", "Initializing GDT (Global Descriptor Table)...");
	gdt_init();

	// init interrupts
	LOG_DEBUG("KERNEL", "Initializing interrupts");
	interrupts_init_result_t interrupts_result = interrupts_init();
	char *interrupts_result_name[3] = {
		[INTERRUPTS_INIT_OK] = "OK",
		[INTERRUPTS_INIT_MSR_NOT_SUPPORTED] = "MSR_NOT_SUPPORTED",
		[INTERRUPTS_INIT_APIC_NOT_SUPPORTED] = "APIC_NOT_SUPPORTED"};
	if (interrupts_result != 0) {
		char *name = "UNKNOWN";
		if (interrupts_result < 3) name = interrupts_result_name[interrupts_result];
		interrupts_fail("INTERRUPTS_INIT bad return!", interrupts_result, name);
	}

	// __asm__ __volatile__("int $0");

	// init ps/2
	ps2_mouse_init(0);

	// init vma
	LOG_DEBUG("KERNEL", "Virtual Memory Allocator initializing...");
	vma_init();

	// init syscall
	// LOG_DEBUG("KERNEL", "Initializing syscalls...");
	// syscall_init();

	// simple mouse demo
	const char *msg = "xD-DOS (Extended Drive - Disk Operating System)\r\n> https://github.com/jasonchristiandev/xD-DOS\r\n> Maintained by Jason Christian.";
	for (;;) {
		graphics_clear(0xFFFFFF);
		graphics_psf_put_text(fallback_font, msg, 4, 4, 0xFFFFFF, 0x000000);
		graphics_rect(mouse_x, mouse_y, 16, 16, 0xFFFFFF);
		pit_sleep_ms(16);
	}
}