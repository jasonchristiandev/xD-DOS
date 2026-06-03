bits 64
extern xddos_interrupts_exception_handler

%macro isr_no_err_stub 1
global isr_stub_%+%1
isr_stub_%+%1:
	push 0
	push %1
	jmp interrupt_common
%endmacro

%macro isr_err_stub 1
global isr_stub_%+%1
isr_stub_%+%1:
	push %1
	jmp interrupt_common
%endmacro

%assign i 0
%rep 32
	; 8, 10, 11, 12, 13, 14, 17, 30
	%if i == 8 || (i >= 10 && i <= 14) || i == 17 || i == 30
		isr_err_stub i
	%else
		isr_no_err_stub i
	%endif
%assign i i+1
%endrep

interrupt_common:
	push rbp
	push rdi
	push rsi
	push rdx
	push rcx
	push rbx
	push rax
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15

	mov rdi, rsp

	mov rbp, rsp
	and rsp, ~0xF

	call xddos_interrupts_exception_handler

	mov rsp, rbp

	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rax
	pop rbx
	pop rcx
	pop rdx
	pop rsi
	pop rdi
	pop rbp

	add rsp, 16

	iretq

global isr_stub_table
isr_stub_table:
%assign i 0
%rep 32
	dq isr_stub_%+i
%assign i i+1
%endrep