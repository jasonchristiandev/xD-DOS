OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

PHDRS
{
	boot PT_LOAD FLAGS(7);
	text PT_LOAD FLAGS(5);
	rodata PT_LOAD FLAGS(4);
	data PT_LOAD FLAGS(6);
}

SECTIONS
{
	. = 1M;

	.boot : {
		KEEP(*(.multiboot2))
		*(.boot_text)
		*(.boot_data)
		*(.boot_bss)
	} :boot

	. += 0xFFFFFFFF80000000;

	. = ALIGN(CONSTANT(MAXPAGESIZE));

	.text : AT(ADDR(.text) - 0xFFFFFFFF80000000) {
		*(.text .text.*)
	} :text

	. = ALIGN(CONSTANT(MAXPAGESIZE));

	.rodata : AT(ADDR(.rodata) - 0xFFFFFFFF80000000) {
		*(.rodata .rodata.*)
	} :rodata

	.note.gnu.build-id : AT(ADDR(.note.gnu.build-id) - 0xFFFFFFFF80000000) {
		*(.note.gnu.build-id)
	} :rodata

	. = ALIGN(CONSTANT(MAXPAGESIZE));

	.data : AT(ADDR(.data) - 0xFFFFFFFF80000000) {
		*(.data .data.*)
	} :data

	.bss : AT(ADDR(.bss) - 0xFFFFFFFF80000000) {
		*(.bss .bss.*)
		*(COMMON)
	} :data

	/DISCARD/ : {
		*(.eh_frame*)
		*(.note .note.*)
	}
}