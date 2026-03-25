#include <stdio.h>
#include <omp.h>


int main(void) {
    omp_set_num_threads(3);
#pragma omp parallel
{
    printf("Thread %d: in parallel region\n", omp_get_thread_num());

    #pragma omp single
    {
        printf("Only one thread prints this (ID %d)\n",
               omp_get_thread_num());
    }

    // all threads continue here after the implicit barrier
     printf("Thread %d: in parallel region\n", omp_get_thread_num());
}
return 0;
}
