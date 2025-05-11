[bits 32]

global outb
global outw
global inb
global inw

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

outw:			; arguments: port, value
	push ebp
	mov ebp, esp

	mov dx, [ebp+8]
	mov ax, [ebp+12]

	out dx, ax

	pop ebp

	ret

inw:			; arguments: port, return: (stored in ax) value
	push ebp
	mov ebp, esp

	mov dx, [ebp + 8]
	
	in ax, dx

	pop ebp

	ret
