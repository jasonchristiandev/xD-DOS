global vmm_switch_stack
vmm_switch_stack:
	mov rsp, rdi
	call rsi
	hlt