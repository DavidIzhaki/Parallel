Build & Run Notes (Linux/Codespaces, x86-64)
Check CPU features (optional)
grep -m1 -o 'avx[^ ]*' /proc/cpuinfo
# expect: avx avx2 (maybe avx512…)
Files we have
c_intrinsics.c — scalar → SIMD with intrinsics (SSE/AVX)
c_asm_avx.s — AVX (v*) assembly with main
mmul2x2_avx.s — AVX function (no main)
mmul2x2_intrin.c — intrinsics driver for the 2×2 mmul
mmul2x2_avx_standalone.s — AVX 2×2 mmul with main (pure asm demo)
1) Intrinsics demo (scalar example)
gcc -O3 -Wall -msse2 c_intrinsics.c -o c_intrinsics
./c_intrinsics
# Result of (8.00 + 2) / (1.50 * 4) = 1.6667
(Use -mavx if your file uses AVX intrinsics.)
2) Assembly demo (scalar example, AVX)
gcc -O3 -no-pie -mavx c_asm_avx.s -o c_asm_avx
./c_asm_avx
# Assembly (AVX): (8.00 + 2) / (1.50 * 4) = 1.6667
3) 2×2 matrix multiply — intrinsics + asm (linked together)
# assemble the function
gcc -O3 -no-pie -mavx mmul2x2_avx.s -c -o mmul2x2_avx.o
# build the intrinsics driver and link
gcc -O3 -Wall -mavx -no-pie mmul2x2_intrin.c mmul2x2_avx.o -o mmul2x2_demo
./mmul2x2_demo
# C  intrin: [19 22; 43 50]
# ASM  avx : [19 22; 43 50]
