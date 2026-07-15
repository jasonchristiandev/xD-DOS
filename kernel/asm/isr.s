bits 64
extern interrupts_exception_handler

%assign i 0
%rep 256
	global isr_stub_%+i
	isr_stub_%+i:
		; 8, 10, 11, 12, 13, 14, 17, 30
		%if !(i == 8 || (i >= 10 && i <= 14) || i == 17 || i == 30)
			push 0
		%endif
		push i
		jmp isr_handler
	%assign i i+1
%endrep

global isr_stub_table
isr_stub_table:
%assign i 0
%rep 256
	dq isr_stub_%+i
%assign i i+1
%endrep

isr_handler:
	; uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
	; uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp;

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
	and rsp, ~0xF ; alignment stuff that i dont understand

	call interrupts_exception_handler

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

	add rsp, 16; error code and vector number

	iretq
