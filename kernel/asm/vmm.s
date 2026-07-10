global xddos_vmm_switch_stack
xddos_vmm_switch_stack:
	mov rsp, rdi
	call rsi
	hlt