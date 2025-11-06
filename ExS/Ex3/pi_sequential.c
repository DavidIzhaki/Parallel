// pi_sequential.c — Sequential Monte Carlo estimate of π with timing (timespec_get)
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static inline double urand01(void) {
    // Uniform in [0,1)
    return (double)rand() / (RAND_MAX + 1.0);
}

static inline double timespec_diff_sec(struct timespec a, struct timespec b) {
    // Return (b - a) in seconds
    long sec  = b.tv_sec  - a.tv_sec;
    long nsec = b.tv_nsec - a.tv_nsec;
    return (double)sec + (double)nsec * 1e-9;
}

int main(int argc, char **argv) {
    // Number of random samples (default: 10,000,000)
    uint64_t n = (argc > 1) ? strtoull(argv[1], NULL, 10) : 10000000ULL;

    // RNG seed (use srand(1234) for reproducibility in class)
    srand((unsigned)time(NULL));

    uint64_t inside = 0;
    struct timespec t0, t1;

    // Start timing the sampling loop
    timespec_get(&t0, TIME_UTC);

    for (uint64_t i = 0; i < n; ++i) {
        double x = urand01();
        double y = urand01();
        if (x * x + y * y <= 1.0)
            inside++;
    }

    // Stop timing
    timespec_get(&t1, TIME_UTC);

    double elapsed = timespec_diff_sec(t0, t1);
    double pi_est  = 4.0 * (double)inside / (double)n;
    double speed   = (elapsed > 0.0) ? (double)n / elapsed : 0.0;

    printf("Samples: %llu\n", (unsigned long long)n);
    printf("Inside:  %llu\n", (unsigned long long)inside);
    printf("pi ≈ %.10f (abs err %.10f)\n", pi_est, fabs(pi_est - M_PI));
    printf("Time: %.3f ms (%.6f s)\n", elapsed * 1e3, elapsed);
    printf("Throughput: %.2f samples/s\n", speed);
    return 0;
}
