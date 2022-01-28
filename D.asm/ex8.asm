global _start

_start:
	call func
	mov eax, 1
	int 0x80

func:
	push ebp
	mov ebp, esp
	sub esp, 3 
	mov [esp+0], byte 'H'
	mov [esp+1], byte 'i'
	mov [esp+2], byte 0x00
	mov eax, 4
	mov ebx, 1
	mov ecx, esp
	mov edx, 3
	int 0x80
	mov esp, ebp
	pop ebp
	ret
