#include <stdio.h>
#include <math.h>
#include <omp.h>

double seq_euler_mascheroni(long long N) {
    double sum = 0.0;

    for (long long i = 1; i < N; i++) {
        sum += 1.0 / (double)i;
    }

    return log((double)N) - sum;
}

double omp_euler_mascheroni(long long N) {
    double sum = 0.0;

    #pragma omp parallel
    {
        /* Get the number of threads */
        const int num_threads = omp_get_num_threads();

        /* Cyclically sum 1/i in each thread */
        double thread_sum = 0.0;
        int tid = omp_get_thread_num() + 1;

        for (long long i = tid; i < N; i += num_threads) {
            thread_sum += 1.0 / (double)i;
        }

        /* Add the thread sum to the global sum, synchronously */
        #pragma omp critical
        sum += thread_sum;
    }

    return log((double)N) - sum;
}

int main(void) {
    long long N;

    printf("Enter N: ");
    if (scanf("%lld", &N) != 1 || N <= 1) {
        printf("Invalid N\n");
        return 1;
    }

    double start, end;
    double seq_val, omp_val;
    double seq_time, omp_time;

    /* Sequential */
    start = omp_get_wtime();
    seq_val = seq_euler_mascheroni(N);
    end = omp_get_wtime();
    seq_time = end - start;

    /* OpenMP */
    start = omp_get_wtime();
    omp_val = omp_euler_mascheroni(N);
    end = omp_get_wtime();
    omp_time = end - start;

    printf("\nSequential result : %.15f\n", seq_val);
    printf("Sequential time   : %f seconds\n", seq_time);

    printf("\nOpenMP result     : %.15f\n", omp_val);
    printf("OpenMP time       : %f seconds\n", omp_time);

    return 0;
}
