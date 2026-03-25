#include <immintrin.h>
#include <stdio.h>

int main(void) {
    /*
      Each vector stores two complex numbers as:
      [real0, imag0, real1, imag1]

      vec1 = [(4 + 5i), (13 + 6i)]
      vec2 = [(9 + 3i), (6 + 7i)]
    */
    __m256d vec1 = _mm256_setr_pd(4.0, 5.0, 13.0, 6.0);
    __m256d vec2 = _mm256_setr_pd(9.0, 3.0,  6.0, 7.0);

    /* Sign mask used to negate the imaginary terms when needed */
    __m256d neg = _mm256_setr_pd(1.0, -1.0, 1.0, -1.0);

    /*
      Step 1: element-wise multiplication
      vec3 = [a0*c0, b0*d0, a1*c1, b1*d1]
           = [4*9,   5*3,   13*6,  6*7]
           = [36,    15,    78,    42]
    */
    __m256d vec3 = _mm256_mul_pd(vec1, vec2);

    /*
      Step 2: swap real/imaginary parts of vec2 inside each complex pair
      vec2 = [d0, c0, d1, c1]
           = [3, 9, 7, 6]
    */
    vec2 = _mm256_permute_pd(vec2, 0x5);

    /*
      Step 3: negate the second lane of each pair
      vec2 = [d0, -c0, d1, -c1]
           = [3, -9, 7, -6]
    */
    vec2 = _mm256_mul_pd(vec2, neg);

    /*
      Step 4: multiply again
      vec4 = [a0*d0, b0*(-c0), a1*d1, b1*(-c1)]
           = [4*3,   5*(-9),   13*7,   6*(-6)]
           = [12,   -45,       91,    -36]
    */
    __m256d vec4 = _mm256_mul_pd(vec1, vec2);

    /*
      Step 5: real parts = ac - bd
      realPart = [36-12, 15-(-45), 78-91, 42-(-36)]
               = [24, 60, -13, 78]
    */
    __m256d realPart = _mm256_hsub_pd(vec3, vec4);

    /*
      Step 6: imaginary parts = ad + bc
      imagPart = [36+12, 15+(-45), 78+91, 42+(-36)]
               = [48, -30, 169, 6]
    */
    __m256d imagPart = _mm256_hadd_pd(vec3, vec4);

    /*
      Step 7: blend the needed lanes
      We want:
      [real0, imag0, real1, imag1]
      = [24, -30, -13, 6]
    */
    __m256d result = _mm256_blend_pd(realPart, imagPart, 0b1010);
    result = _mm256_permute4x64_pd(result, 0b11011000);

    /* Print result */
    double *res = (double *)&result;
    printf("%lf %lf %lf %lf\n", res[0], res[1], res[2], res[3]);

 
    return 0;
}
