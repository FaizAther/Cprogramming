global _start

section .data
	addr db "yellow"

section .text
_start:
	; sys_write
	mov eax, 4
	mov ebx, 1
	mov ecx, addr
	mov edx, 6
	int 0x80
	; sys_exit
	mov eax, 1
	mov ebx, 0
	int 0x80
