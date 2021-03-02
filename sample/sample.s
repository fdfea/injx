inject:
    push %rax
    push %rdi
    push %rsi
    push %rdx
    movl $1, %eax                # system call SYS_write
    movl $1, %edi                # first argument  = stdout
    leal string(%rip), %esi      # second argument = string
    movl $18, %edx               # third argument  = length of string
    syscall
    pop %rdx
    pop %rsi
    pop %rdi
    pop %rax
    retq
string: 
    .asciz "I'm in your code!\n"

