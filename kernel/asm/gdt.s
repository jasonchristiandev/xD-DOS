global gdt_flush

section .text
bits 64

gdt_flush:
	; data segment
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	; code segment
	pop rdi
	mov rax, 0x08
	push rax
	push rdi
	retfq