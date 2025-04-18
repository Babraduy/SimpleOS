[bits 32]
[extern main]

global _start_kernel_entry

section .text
_start_kernel_entry:
	call main

hang:
	hlt
	jmp hang
