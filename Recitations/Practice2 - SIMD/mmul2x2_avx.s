    .text
    .globl mmul2x2_f32_avx
    .type  mmul2x2_f32_avx, @function

# void mmul2x2_f32_avx(const float* A, const float* B, float* C);
# A: rdi (row-major: a00,a01,a10,a11)
# B: rsi (row-major: b00,b01,b10,b11)
# C: rdx (row-major: c00,c01,c10,c11)

mmul2x2_f32_avx:
    # Load A = [a00 a01 a10 a11]
    vmovups   (%rdi),   %xmm0

    # Build row vectors of A
    vpermilps $0x44, %xmm0, %xmm1       # xmm1 = [a00 a01 a00 a01]
    vpermilps $0xEE, %xmm0, %xmm2       # xmm2 = [a10 a11 a10 a11]

    # Load B = [b00 b01 b10 b11]
    vmovups   (%rsi),   %xmm3

    # Rearrange B to [b00 b10 b01 b11]
    vpermilps $0xD8, %xmm3, %xmm4       # xmm4 = [b00 b10 b01 b11]

    # Compute first row of C: [c00 c01 c00 c01]
    vmulps    %xmm1, %xmm4, %xmm5       # [a00*b00 a01*b10 a00*b01 a01*b11]
    vhaddps   %xmm5, %xmm5, %xmm5       # [c00 c01 c00 c01]

    # Store c00, c01
    vmovss    %xmm5,   (%rdx)           # C[0,0]
    vextractps $1, %xmm5, 4(%rdx)       # C[0,1]

    #Could be done like this too. 
    #vmovss    %xmm5, (%rdx)              # C[0,0]
    #vshufps   $0x55, %xmm5, %xmm5, %xmm7 # xmm7 low lane = c01
    #vmovss    %xmm7, 4(%rdx)             # C[0,1]

    # Compute second row of C: [c10 c11 c10 c11]
    vmulps    %xmm2, %xmm4, %xmm6       # [a10*b00 a11*b10 a10*b01 a11*b11]
    vhaddps   %xmm6, %xmm6, %xmm6       # [c10 c11 c10 c11]

    # Store c10, c11
    vmovss    %xmm6,   8(%rdx)          # C[1,0]
    vextractps $1, %xmm6, 12(%rdx)      # C[1,1]

    ret

    .section .note.GNU-stack,"",@progbits
