#include <stdio.h>
#include <immintrin.h>

void mmul2x2_f32_intrin(const float *A, const float *B, float *C) {
    // A = [a00 a01 a10 a11]
    __m128 a = _mm_loadu_ps(A);

    // Build row vectors of A
    __m128 row0 = _mm_permute_ps(a, 0x44); // [a00 a01 a00 a01]
    __m128 row1 = _mm_permute_ps(a, 0xEE); // [a10 a11 a10 a11]

    // B = [b00 b01 b10 b11]
    __m128 b = _mm_loadu_ps(B);

    // Rearrange B to [b00 b10 b01 b11]
    __m128 bmix = _mm_permute_ps(b, 0xD8); // [b00 b10 b01 b11]

    // First output row: [c00 c01 c00 c01]
    __m128 p0 = _mm_mul_ps(row0, bmix);
    p0 = _mm_hadd_ps(p0, p0);

    // Store c00, c01
    _mm_store_ss(&C[0], p0);                          // C[0] = c00
    __m128 p0_hi = _mm_permute_ps(p0, 0x55);         // low lane = c01
    _mm_store_ss(&C[1], p0_hi);                      // C[1] = c01

    // Second output row: [c10 c11 c10 c11]
    __m128 p1 = _mm_mul_ps(row1, bmix);
    p1 = _mm_hadd_ps(p1, p1);

    // Store c10, c11
    _mm_store_ss(&C[2], p1);                         // C[2] = c10
    __m128 p1_hi = _mm_permute_ps(p1, 0x55);         // low lane = c11
    _mm_store_ss(&C[3], p1_hi);                      // C[3] = c11
}

extern void mmul2x2_f32_avx(const float*, const float*, float*);

int main(void) {
    float A[4] = {1, 2, 3, 4};
    float B[4] = {5, 6, 7, 8};
    float C1[4] = {0};
    float C2[4] = {0};

    mmul2x2_f32_intrin(A, B, C1);
    mmul2x2_f32_avx(A, B, C2);

    printf("C intrin: [%g %g; %g %g]\n", C1[0], C1[1], C1[2], C1[3]);
    printf("ASM avx : [%g %g; %g %g]\n", C2[0], C2[1], C2[2], C2[3]);
    return 0;
}
