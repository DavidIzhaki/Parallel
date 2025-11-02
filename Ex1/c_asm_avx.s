    .text

# double cfunction(int i, double d, long l, float f)
    .globl cfunction
    .type  cfunction, @function
cfunction:
    # (d + i) -> XMM0
    vcvtsi2sd   %edi, %xmm2, %xmm2
    vaddsd      %xmm2, %xmm0, %xmm0
    # (f * l) -> XMM1
    vcvtsi2ss   %rsi, %xmm2, %xmm2
    vmulss      %xmm2, %xmm1, %xmm1
    # to double & divide
    vcvtss2sd   %xmm1, %xmm1, %xmm1
    vdivsd      %xmm1, %xmm0, %xmm0
    ret

# int main(void)
    .globl main
    .type  main, @function
main:
    push   %rbp
    mov    %rsp, %rbp
    sub    $16, %rsp                    # keep 16B alignment before every call

    mov    $2,  %edi                    # i
    movsd  .Ld(%rip), %xmm0             # d = 8
    mov    $4,  %rsi                    # l
    movss  .Lf(%rip), %xmm1             # f = 1.5
    call   cfunction                    # result -> xmm0

    lea    .Lfmt(%rip), %rdi            # printf format
    mov    $1, %eax                     # one FP vararg in XMM regs (xmm0)
    call   printf

    xor    %eax, %eax
    add    $16, %rsp
    pop    %rbp
    ret

    .section .rodata
.Ld:   .double 8
.Lf:   .float  1.5
.Lfmt: .string "Assembly (AVX): (8.00 + 2) / (1.50 * 4) = %.4f\n"

    .section .note.GNU-stack,"",@progbits
