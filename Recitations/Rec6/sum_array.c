#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define SIZE 1000000000   /* array size */
#define N    64        /* number of threads */

/* Sequential sum of an array */
int sum_arr(int *arr, int size) {
    int result = 0;

    for (int i = 0; i < size; i++) {
        result += arr[i];
    }

    return result;
}

/* Sum the elements of the array using OpenMP */
int omp_sum_arr(int *arr, int size) {
    int thread_num;
    int partial_sums[N] = {0};

    omp_set_num_threads(N);

    #pragma omp parallel
    {
        #pragma omp single
        thread_num = omp_get_num_threads();

        int id = omp_get_thread_num();

        for (int i = id; i < size; i += thread_num) {
            partial_sums[id] += arr[i];
        }
    }

    /* Sum the partial sums */
    int result = 0;

    for (int i = 0; i < thread_num; i++) {
        result += partial_sums[i];
    }

    return result;
}

int main(void) {
    int *arr = (int *)malloc(SIZE * sizeof(int));
    if (!arr) {
        fprintf(stderr, "Error: malloc failed\n");
        return 1;
    }

    srand((unsigned)time(NULL));
    for (int i = 0; i < SIZE; i++) {
        arr[i] = rand();
    }

    /* Print test statistics */
    printf("Number of threads: %d\n", N);
    printf("Array size: %d\n", SIZE);

    /* Check the sequential performance */
    double start = omp_get_wtime();
    int seq_result = sum_arr(arr, SIZE);
    double end = omp_get_wtime();
    printf("Sequential time: %f\n", end - start);

    /* Check the parallel performance */
    start = omp_get_wtime();
    int omp_result = omp_sum_arr(arr, SIZE);
    end = omp_get_wtime();

    if (seq_result != omp_result) {
        printf("Error: sequential result is different from OpenMP result\n");
        free(arr);
        return 1;
    }

    printf("OpenMP time: %f\n", end - start);

    /* Clean up */
    free(arr);
    return 0;
}
