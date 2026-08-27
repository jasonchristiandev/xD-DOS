%define MULTIBOOT2_HEADER_MAGIC 0xE85250D6
%define MULTIBOOT_ARCHITECTURE_I386 0

section .multiboot2 alloc align=8
mbheader_start:
	dd MULTIBOOT2_HEADER_MAGIC
	dd MULTIBOOT_ARCHITECTURE_I386
	dd mbheader_end - mbheader_start
	dd -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + (mbheader_end - mbheader_start))

	; framebuffer
	align 8
	dw 5 ; MULTIBOOT_HEADER_TAG_FRAMEBUFFER
	dw 1 ; optional
	dd 20 ; 20 bytes
	dd 1024 ; width
	dd 768 ; height
	dd 32 ; depth/bpp

	; memmap
	align 8
	dw 1 ; MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST
	dw 0 ; required
	dd 12 ; 12 bytes
	dd 6 ; MULTIBOOT_TAG_TYPE_MMAP

	; end
	align 8
	dw 0 ; type 0
	dw 0 ; flag 0
	dd 8 ; 8 bytes
mbheader_end:

section .boot_bss nobits alloc write align=16
align 16
stack_bottom:
	resb 16384 ; 16 kb stack
stack_top:

; 4 kb for page table
align 4096
pml4:
	resb 4096
pdpt:
	resb 4096
pd:
	resb 4096

section .boot_data alloc write align=8
align 8
gdt_start:
	dq 0x0000000000000000 ; null
	dq 0x0020980000000000 ; kernel code
	dq 0x0000920000000000 ; kernel data
gdt_end:

gdt_ptr:
	dw gdt_end - gdt_start - 1
	dq gdt_start

section .boot_text alloc exec align=16
[BITS 32]
global _start
_start:
	cli
	mov esp, stack_top

	; multiboot2 info
	mov ebp, eax
	mov esi, ebx

	; clear page table
	mov edi, pml4
	mov ecx, 3072
	xor eax, eax
	cld
	rep stosd

	; pml4 -> pdpt
	mov eax, pdpt
	or eax, 0x3
	mov [pml4], eax
	mov [pml4 + 511 * 8], eax

	; pdpt -> pd
	mov eax, pd
	or eax, 0x3
	mov [pdpt], eax
	mov [pdpt + 510 * 8], eax

	; huge page for pd (0 mb - 2 mb)
	mov eax, 0x00000083
	mov [pd], eax

	; huge page for pd (2 mb - 4 mb)
	mov eax, 0x00200083
	mov [pd + 8], eax

	; load to cr3
	mov eax, pml4
	mov cr3, eax

	; enable PAE, OSFXSR, and OSXMMEXCPT
	mov eax, cr4
	or eax, 0x620
	mov cr4, eax

	; enable long mode
	mov ecx, 0xC0000080
	rdmsr
	or eax, 0x100
	wrmsr

	; set paging to on cr0
	mov eax, cr0
	or eax, 0x80000001
	mov cr0, eax

	; load gdt
	lgdt [gdt_ptr]

	; jump to code segment
	push 0x08
	push longentry
	retf

[BITS 64]
longentry:
	; reload data segment
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	mov rsp, stack_top
	and rsp, -16

	; multiboot2 info
	mov edi, ebp
	mov esi, esi

	cld
	extern boot_main
	call boot_main

.hang:
	cli
	hlt
	jmp .hang