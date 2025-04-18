[bits 32]

global outb
global inb

section .text:
outb:			; arguments: port, value
	push ebp
	mov ebp, esp

	mov dx, [ebp+8]
	mov al, [ebp+12]

	out dx, al

	pop ebp

	ret

inb:			; arguments: port, return: (stored in al) value
	push ebp
	mov ebp, esp

	mov dx, [ebp + 8]
	
	in al, dx

	pop ebp

	ret
