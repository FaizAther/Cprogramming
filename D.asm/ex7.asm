global _start

_start:
	call func	; pushes EIP to stack
			; i.e. the return point then jumps
	mov eax, 1
	int 0x80

func:
	mov ebx, 42
	pop eax		; pop the return point to eax
	jmp eax
