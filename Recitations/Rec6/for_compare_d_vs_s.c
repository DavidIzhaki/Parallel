#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define MAX         10000     // number of iterations (try changing this)
#define NUM_THREADS 8          // how many threads to use
#define CHUNK       4          // chunk size for dynamic

// Heavy work that grows with j (and hence with i)
double big_calc(int j) {
    // More work for larger j
    int reps = j * 500;        // try 200, 500, 1000 to see bigger gaps
    double x = 0.0;

    for (int k = 0; k < reps; k++) {
        x += sqrt(0.123 * k + j);
    }

    return x;
}

int main(void) {
    double *A = malloc(MAX * sizeof(double));
    if (!A) {
        fprintf(stderr, "Failed to allocate A\n");
        return 1;
    }

    omp_set_num_threads(NUM_THREADS);
    printf("MAX = %d, NUM_THREADS = %d, CHUNK = %d\n\n",
           MAX, NUM_THREADS, CHUNK);

    double start, end;
    double checksum_static = 0.0;
    double checksum_dynamic = 0.0;

    // ---- Static schedule ----
    start = omp_get_wtime();

    #pragma omp parallel for schedule(static) reduction(+:checksum_static)
    for (int i = 0; i < MAX; i++) {
        int j = 5 + 2 * (i + 1);       // grows with i
        double val = big_calc(j);
        A[i] = val;
        checksum_static += val;        // avoid optimizing away the work
    }

    end = omp_get_wtime();
    double static_time = end - start;
    printf("Static schedule time   : %f seconds (checksum = %.3f)\n",
           static_time, checksum_static);

    // ---- Dynamic schedule ----
    start = omp_get_wtime();

    #pragma omp parallel for schedule(dynamic, CHUNK) reduction(+:checksum_dynamic)
    for (int i = 0; i < MAX; i++) {
        int j = 5 + 2 * (i + 1);       // same work pattern
        double val = big_calc(j);
        A[i] = val;
        checksum_dynamic += val;
    }

    end = omp_get_wtime();
    double dynamic_time = end - start;
    printf("Dynamic schedule time  : %f seconds (checksum = %.3f)\n",
           dynamic_time, checksum_dynamic);

    free(A);
    return 0;
}
