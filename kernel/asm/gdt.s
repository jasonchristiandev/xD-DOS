global xddos_gdt_flush

section .text
bits 64

xddos_gdt_flush:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	push qword 0x08
	
	lea rax, [rel .reload_cs]
	push rax
	
	retfq

.reload_cs:
	ret