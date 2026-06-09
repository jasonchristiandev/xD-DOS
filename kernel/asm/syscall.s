global xddos_syscall_entry
global global_kernel_stack
extern xddos_syscall_handler

section .data
	global_user_stack_backup: dq 0x0
	global_kernel_stack: dq 0x0 

section .text
bits 64

xddos_syscall_entry:
	mov [rel global_user_stack_backup], rsp
	mov rsp, [rel global_kernel_stack]

	push r11
	push rcx
	
	push rbp
	push rbx
	push r12
	push r13
	push r14
	push r15
	
	mov rcx, rdx
	mov rdx, rsi
	mov rsi, rdi
	mov rdi, rax

	call xddos_syscall_handler

	pop r15
	pop r14
	pop r13
	pop r12
	pop rbx
	pop rbp
	
	pop rcx
	pop r11

	mov rsp, [rel global_user_stack_backup]

	sysret