    .text
    .globl mmul2x2_f32_avx
    .type  mmul2x2_f32_avx, @function
# void mmul2x2_f32_avx(const float* A, const float* B, float* C);
# A: rdi (row-major: a00,a01,a10,a11)
# B: rsi (row-major: b00,b01,b10,b11)
# C: rdx (row-major: c00,c01,c10,c11)
mmul2x2_f32_avx:
    # Load rows of A
    vmovups   (%rdi),   %xmm0           # [a00 a01 a10 a11]

    # Make row0(A) and row1(A) in separate regs (low lanes hold the pair)
    vpermilps $0x44, %xmm0, %xmm1       # xmm1 = [a00 a01 a00 a01]
    vpermilps $0xEE, %xmm0, %xmm2       # xmm2 = [a10 a11 a10 a11]

    # Load rows of B
    vmovups   (%rsi),   %xmm3           # [b00 b01 b10 b11]

    # Build columns of B: col0=[b00 b10 ..], col1=[b01 b11 ..]
    vpermilps $0x88, %xmm3, %xmm4       # xmm4 = [b00 b10 b00 b10]
    vpermilps $0xDD, %xmm3, %xmm5       # xmm5 = [b01 b11 b01 b11]

    # c00 = dot( row0(A), col0(B) )
    vmulps    %xmm1, %xmm4, %xmm6       # [a00*b00 a01*b10 ...]
    vhaddps   %xmm6, %xmm6, %xmm6       # pairwise sum
    vhaddps   %xmm6, %xmm6, %xmm6       # reduce 2 -> scalar (broadcast)
    vmovss    %xmm6,   (%rdx)           # C[0,0]

    # c01 = dot( row0(A), col1(B) )
    vmulps    %xmm1, %xmm5, %xmm6
    vhaddps   %xmm6, %xmm6, %xmm6
    vhaddps   %xmm6, %xmm6, %xmm6
    vmovss    %xmm6,   4(%rdx)          # C[0,1]

    # c10 = dot( row1(A), col0(B) )
    vmulps    %xmm2, %xmm4, %xmm6
    vhaddps   %xmm6, %xmm6, %xmm6
    vhaddps   %xmm6, %xmm6, %xmm6
    vmovss    %xmm6,  8(%rdx)           # C[1,0]

    # c11 = dot( row1(A), col1(B) )
    vmulps    %xmm2, %xmm5, %xmm6
    vhaddps   %xmm6, %xmm6, %xmm6
    vhaddps   %xmm6, %xmm6, %xmm6
    vmovss    %xmm6, 12(%rdx)           # C[1,1]

    ret

    .section .note.GNU-stack,"",@progbits
