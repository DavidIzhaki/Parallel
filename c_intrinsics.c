#include <stdio.h>
#include <immintrin.h>   // SSE / AVX intrinsics

//---------------------------------------------
// Compute: (d + i) / (f * l)
//---------------------------------------------
double cfunction(int i, double d, long l, float f) {
    // (d + i)
    __m128d d_vec  = _mm_set_sd(d);                        // load d
    __m128d i_vec  = _mm_cvtsi32_sd(_mm_setzero_pd(), i);  // int -> double
    __m128d sum    = _mm_add_sd(d_vec, i_vec);             // (d + i)

    // (f * l)
    __m128  f_vec  = _mm_set_ss(f);                        // load f
    __m128  l_vec  = _mm_cvtsi64_ss(_mm_setzero_ps(), l);  // long -> float
    __m128  prod   = _mm_mul_ss(f_vec, l_vec);             // (f * l)

    // convert (f*l) to double
    __m128d denom  = _mm_cvtss_sd(_mm_setzero_pd(), prod);

    // divide (d + i) / (f * l)
    __m128d result = _mm_div_sd(sum, denom);

    // extract result
    return _mm_cvtsd_f64(result);
}

//---------------------------------------------
// Main function
//---------------------------------------------
int main(void) {
    int i = 2;
    double d = 8.0;
    long l = 4;
    float f = 1.5f;

    double out = cfunction(i, d, l, f);

    printf("Result of (%.2f + %d) / (%.2f * %ld) = %.4f\n",
           d, i, f, l, out);

    return 0;
}
