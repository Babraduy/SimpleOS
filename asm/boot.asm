[org 0x7c00]

CODE_SEG equ code_descriptor - GDT_Start
DATA_SEG equ data_descriptor - GDT_Start

KERNEL_LOCATION equ 0x1000
BOOT_DISK db 0
KERNEL_SECTORS equ 50 ; from script 'calculate_kernel_size'

mov [BOOT_DISK], dl

global _start

section .text
_start:
	xor ax, ax			; set ax to 0
	mov es, ax			; to set es to 0
	mov ds, ax			; to also set ds to 0
	mov bp, 0x8000
	mov sp, bp

	mov ah, 2			; to read sectors
	mov ch, 0			; cylinder number
	mov cl, 2			; sector number

	mov al, KERNEL_SECTORS		; read the whole kernel

	mov dh, 0			; set dh (number of head) to 0
	mov dl, [BOOT_DISK]		; boot disk number
	mov bx, KERNEL_LOCATION		; set bx to kernel location
	int 0x13			; interrupt

	jc DISK_ERROR

	mov ah, 0			; clear the screen
	mov al, 0x3
	int 0x10

	cli				; clear interrupts
	lgdt [GDT_Descriptor]		; load GDT

	mov eax, cr0			; set the last bit of cr0 to 1
	or eax, 1
	mov cr0, eax

	jmp CODE_SEG:start_protected_mode	; jump to start_protected_mode
	
	jmp $

print_real_mode:
	mov al, [bx]
	cmp al, 0

	je .print_end

	mov ah, 0x0e
	int 0x10

	inc bx
	jmp print_real_mode

.print_end:
	ret

DISK_ERROR:
	DISK_ERROR_MSG db "DISK ERROR!", 0
	mov bx, DISK_ERROR_MSG
	call print_real_mode

	jmp $

GDT_Start:				; the GDT (Global Descriptor Table) start position
	null_descriptor:
		dd 0
		dd 0
	code_descriptor:
		dw 0xffff		; limit
		dw 0			; base
		db 0
		db 0b10011010		; p,p,t and Type flags
		db 0b11001111		; other flags + limit (last four bits)
		db 0			; last 4 bits of base

	data_descriptor:
		dw 0xffff		; limit
		dw 0			; base
		db 0
		db 0b10010010		; p,p,t and Type flags
		db 0b11001111		; other flags + limit (last four bits)
		db 0			; last 4 bits of base
GDT_End:
	

GDT_Descriptor:
	dw GDT_End - GDT_Start - 1	; size
	dd GDT_Start			; start

[bits 32]
start_protected_mode:
	mov ax, DATA_SEG		; set ALL registers equal to the DATA_SEG location
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ebp, 0x90000		; set up the stack
	mov esp, ebp

	jmp KERNEL_LOCATION		; jump to the kernel

times 510-($-$$) db 0
db 0x55, 0xaa
