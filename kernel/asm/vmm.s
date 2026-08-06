global vmm_switch_stack
vmm_switch_stack:
	mov rsp, rdi
	and rsp, -16
	call rsi
.hang:
	cli
	hlt
	jmp .hang