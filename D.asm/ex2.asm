global _start

section .data
	msg db "Hello, World!", 0x000a
	len equ $ - msg

section .text
_start:
	; write msg
	mov eax, 4
	mov ebx, 1
	mov ecx, msg
	mov edx, len
	int 0x80
	; write msg again
	mov eax, 4
	mov ebx, 1
	mov ecx, msg
	mov edx, len
	int 0x80
	; exit
	mov eax, 1
	mov ebx, 0
	int 0x80
