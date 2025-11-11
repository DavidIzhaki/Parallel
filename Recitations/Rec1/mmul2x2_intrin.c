#include <stdio.h>
#include <immintrin.h>

void mmul2x2_f32_intrin(const float *A, const float *B, float *C) {
    // A = [a00 a01 a10 a11]
    __m128 a = _mm_loadu_ps(A);

    // Make row0 and row1 replicated into low lanes
    __m128 row0 = _mm_permute_ps(a, 0x44); // [a00 a01 a00 a01]
    __m128 row1 = _mm_permute_ps(a, 0xEE); // [a10 a11 a10 a11]

    // B = [b00 b01 b10 b11]
    __m128 b = _mm_loadu_ps(B);

    // Columns of B
    __m128 col0 = _mm_permute_ps(b, 0x88); // [b00 b10 b00 b10]
    __m128 col1 = _mm_permute_ps(b, 0xDD); // [b01 b11 b01 b11]

    // c00
    __m128 p = _mm_mul_ps(row0, col0);
    p = _mm_hadd_ps(p, p);
    p = _mm_hadd_ps(p, p);
    _mm_store_ss(&C[0], p);

    // c01
    p = _mm_mul_ps(row0, col1);
    p = _mm_hadd_ps(p, p);
    p = _mm_hadd_ps(p, p);
    _mm_store_ss(&C[1], p);

    // c10
    p = _mm_mul_ps(row1, col0);
    p = _mm_hadd_ps(p, p);
    p = _mm_hadd_ps(p, p);
    _mm_store_ss(&C[2], p);

    // c11
    p = _mm_mul_ps(row1, col1);
    p = _mm_hadd_ps(p, p);
    p = _mm_hadd_ps(p, p);
    _mm_store_ss(&C[3], p);
}

int main(void) {
    float A[4] = {1, 2, 3, 4};
    float B[4] = {5, 6, 7, 8};
    float C1[4] = {0};
    float C2[4] = {0};

    // Intrinsics
    mmul2x2_f32_intrin(A, B, C1);

    // Assembly (declare prototype and call it if you assembled it)
    extern void mmul2x2_f32_avx(const float*, const float*, float*);
    mmul2x2_f32_avx(A, B, C2);

    printf("C  intrin: [%g %g; %g %g]\n", C1[0], C1[1], C1[2], C1[3]);
    printf("ASM  avx : [%g %g; %g %g]\n", C2[0], C2[1], C2[2], C2[3]);
    return 0;
}
