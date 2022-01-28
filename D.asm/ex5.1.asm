global _start

section .data
	; bd is 1 byte, 0x00
	addr db "yellow", 0x00
	name1 db "string", 0x00
	name2 db 0xff
	name3 db 100
	; dw is 2 bytes
	name4 dw 1000
	; dd is 4 bytes
	name5 dd 100000

section .text
_start:
	mov [addr], byte 'H'
	mov [addr+5], byte '!'
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
