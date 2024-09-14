section .text
global _start

__via_std_out:
    mov rax, 1          ; syscall number for write
    mov rdi, 1          ; file descriptor 1 (stdout)
    syscall
    ret

__via_std_in:
    mov rax, 0          ; syscall number for read
    mov rdi, 0          ; file descriptor 0 (stdin)
    syscall
    mov byte [rsi + rax - 1], 0 ; Null-terminate the string
    ret

__via_exit:
    mov rax, 60         ; syscall number for exit
    mov rdi, rdi        ; rdi contains exit code
    syscall
    ret

_main:
    mov rsi, __temp_0
    mov rdx, 60
    call __via_std_out
    mov rsi, __temp_1
    mov rdx, 44
    call __via_std_out
    mov rdi, 1
    call __via_exit
    mov rsi, __temp_2
    mov rdx, 14
    call __via_std_out
    mov rdi, 0
    call __via_exit
    ret

_start:
    call _main

section .data
    var dd 10 ; local var = 10


    __temp_0 db 'test.via:3: [33mwarning: [0mnext line will throw an error', 0xA ; warn('test.via:3: [33mwarning: [0mnext line will throw an error')
    __temp_1 db 'test.via:4: [31merror:   [0merror message', 0xA ; error('test.via:4: [31merror:   [0merror message')
    __temp_2 db 'thiswontprint', 0xA ; print('thiswontprint')