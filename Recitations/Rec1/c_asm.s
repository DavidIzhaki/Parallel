    .text
    .globl cfunction
cfunction:
    cvtsi2sd   %edi, %xmm2
    addsd      %xmm2, %xmm0
    cvtsi2ss   %rsi, %xmm2
    mulss      %xmm2, %xmm1
    cvtss2sd   %xmm1, %xmm1
    divsd      %xmm1, %xmm0
    ret

    .globl main
main:
    push   %rbp
    mov    %rsp, %rbp
    sub    $8, %rsp                # 16B-align stack before calls

    mov    $2,  %edi               # i
    movsd  .Ld(%rip), %xmm0        # d = 8.0
    mov    $4,  %rsi               # l
    movss  .Lf(%rip), %xmm1        # f = 1.5
    call   cfunction               # result in xmm0

    mov    $.Lfmt, %rdi            # printf format
    mov    $1, %eax                # <-- number of XMM/SSE args (xmm0)
    call   printf

    xor    %eax, %eax
    add    $8, %rsp
    leave
    ret

    .section .rodata
.Ld:   .double 8.0
.Lf:   .float  1.5
.Lfmt: .string "Assembly: (8.00 + 2) / (1.50 * 4) = %.4f\n"

    .section .note.GNU-stack,"",@progbits
