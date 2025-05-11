[bits 32]

extern main

global _start_kernel_entry

section .text
_start_kernel_entry:
	call main			; call the main function

hang:					; hang after main function returns
	hlt
	jmp hang
