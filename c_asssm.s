        .text

# double cfunction(int i, double d, long l, float f)
        .globl  _cfunction
_cfunction:
        vcvtsi2sd   %edi, %xmm2, %xmm2     # i -> double
        vaddsd      %xmm2, %xmm0, %xmm0    # xmm0 = d + i
        vcvtsi2ss   %rsi, %xmm2, %xmm2     # l -> float
        vmulss      %xmm2, %xmm1, %xmm1    # xmm1 = f * l
        vcvtss2sd   %xmm1, %xmm1, %xmm1    # (double)(f*l)
        vdivsd      %xmm1, %xmm0, %xmm0    # (d+i)/(f*l)
        ret

# int main(void)
        .globl  _main
_main:
        pushq   %rbp
        movq    %rsp, %rbp

        movl    $2, %edi                 # i
        movsd   _Ld(%rip), %xmm0         # d = 8.0
        movq    $4, %rsi                 # l
        movss   _Lf(%rip), %xmm1         # f = 1.5
        call    _cfunction               # result in xmm0

        # printf("Assembly: ... = %.4f\n", <xmm0>)
        movq    _Lfmt(%rip), %rdi        # fmt in rdi
        movl    $1, %eax                 # number of XMM varargs = 1
        subq    $16, %rsp                # keep stack 16B-aligned at call
        call    _printf
        addq    $16, %rsp

        xorl    %eax, %eax
        leave
        ret

        .section __TEXT,__const
_Ld:    .double 8.0
_Lf:    .float  1.5

        .section __TEXT,__cstring
_Lfmt:  .asciz  "Assembly: (8.00 + 2) / (1.50 * 4) = %.4f\n"
