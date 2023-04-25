; hello.asm
section .data
  msg db "hello, world", 0

section .bss

section .text
  global main

main:
  mov rax, 1      ; 1 = write
  mov rdi, 1      ; 1 = to stdout
  mov rsi, msg    ; msg
  mov rdx, 12     ; length of string without 0
  syscall      ; do it
  mov rax, 60     ; 60 = exit
  mov rdi, 0      ; 0 = success return code
  syscall      ; do it