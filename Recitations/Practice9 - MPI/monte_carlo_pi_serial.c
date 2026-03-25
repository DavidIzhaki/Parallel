// monte_carlo_pi_serial.c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {
    long long N = 1000000;                 // default samples
    if (argc >= 2) N = atoll(argv[1]);     // allow: ./a.out 50000000

    // Seed RNG (time-based)
    srand((unsigned)time(NULL));

    long long hits = 0;
    for (long long i = 0; i < N; i++) {
        double x = rand() / (double)RAND_MAX;
        double y = rand() / (double)RAND_MAX;
        if (x * x + y * y <= 1.0) hits++;
    }

    double pi = 4.0 * (double)hits / (double)N;

    printf("N = %lld\n", N);
    printf("hits = %lld\n", hits);
    printf("pi ≈ %.10f\n", pi);

    return 0;
}
