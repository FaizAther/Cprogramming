global _start

section .data

section .text
_start:
	push byte 65
	sub esp, 4
	mov [esp+0], byte 'H'
	mov [esp+1], byte 'e'
	mov [esp+2], byte 'y'
	mov [esp+3], byte '!'
	; sys_write
	mov eax, 4	; sys_write
	mov ebx, 1	; stdout fd
	mov ecx, esp	; start addr
	mov edx, 5	; nbytes write
	int 0x80
	; sys_exit
	mov eax, 1
	mov ebx, 0
	int 0x80
