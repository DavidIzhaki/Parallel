    .globl cfunction
    .type  cfunction, @function
cfunction:
    # (d + i)
    vcvtsi2sd  %edi, %xmm2, %xmm2       # convert i -> double
    vaddsd     %xmm2, %xmm0, %xmm0      # xmm0 = d + i

    # (f * l)
    vcvtsi2ss  %rsi, %xmm2, %xmm2       # convert l -> float
    vmulss     %xmm2, %xmm1, %xmm1      # xmm1 = f * l

    # convert denominator to double
    vcvtss2sd  %xmm1, %xmm1, %xmm1      # (double)(f*l)

    # divide
    vdivsd     %xmm1, %xmm0, %xmm0      # (d+i)/(f*l)
    ret

#----------------------------------------
# main()
#----------------------------------------
    .globl main
    .type  main, @function
main:
    push   %rbp
    mov    %rsp, %rbp

    mov    $2,  %edi               # i
    movsd  .Ldouble(%rip), %xmm0   # d = 8.0
    mov    $4,  %rsi               # l
    movss  .Lfloat(%rip), %xmm1    # f = 1.5
    call   cfunction

    movsd  %xmm0, -8(%rbp)         # store result
    movsd  -8(%rbp), %xmm0

    # print result
    mov    $.Lformat, %rdi
    mov    $0, %eax
    call   printf

    mov    $0, %eax
    leave
    ret

#----------------------------------------
# Data
#----------------------------------------
    .section .rodata
.Ldouble: .double 8.0
.Lfloat:  .float 1.5
.Lformat: .string "Assembly: (8.00 + 2) / (1.50 * 4) = %.4f\n"
