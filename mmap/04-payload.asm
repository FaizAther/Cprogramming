BITS 64
%define SYS_WRITE 1
%define STDOUT 1

global _start
section .text
_start:
    ;; write
    mov rax, SYS_WRITE
    mov rdi, STDOUT
    lea rsi, [rel hello]
    mov rdx, hello_len
    syscall
    
    mov rax, 60
    xor rdi, rdi
    syscall

section .data
hello: db "Hello, World", 10
hello_len: equ $-hello ;; $ refers to  current address, hello is address
