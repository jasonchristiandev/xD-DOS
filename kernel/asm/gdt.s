default rel
global gdt_flush

section .text
bits 64

gdt_flush:
	lgdt [rdi]

	; data segment
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	; code segment
	mov rax, 0x08
	push rax
	lea rbx, [.reload]
	push rbx

	retfq

.reload:
	ret