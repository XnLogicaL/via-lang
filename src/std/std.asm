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