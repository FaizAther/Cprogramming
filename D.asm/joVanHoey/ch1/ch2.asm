; ch2.asm

section .data
  msg db "Jello, World", 10, 0
  msg_len equ $-msg-1
  msg2 db "Alive n kinkn", 10, 0
  msg2_len equ $-msg2-1
  pi dq 3.14 ; float
  num dq 11.43
  num_n dd 32
  fmt_str db "num_n{%d}, pi{%f}, num{%e}", 10, 0
  fmt_str_len equ $-fmt_str-1

section .bss

section .text
  extern printf
  global main

main:
  mov rbp, rsp; for correct debugging
  push rbp              ; prologue (save base pointer)
  mov rbp, rsp          ; prologue (repleace base pointer with current stack pointer)

  mov rax, 1
  mov rdi, 1
  mov rsi, msg
  mov rdx, msg_len
  syscall

  mov rax, 1
  mov rdi, 1
  mov rsi, msg2
  mov rdx, msg2_len
  syscall

  mov rax, 2 ; 2 xmm reg used
  mov rdi, fmt_str
  mov rsi, [num_n]
  movq xmm0, [pi]
  movq xmm1, [num]
  call printf

  mov rsp, rbp          ; epilogue (base pointer as the stack pointer)
  pop rbp               ; epilogue (revert original base pointer)
  mov rax, 60
  mov edi, 0
  syscall