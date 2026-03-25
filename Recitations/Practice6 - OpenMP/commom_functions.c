#include <omp.h>
#include <stdio.h>

int main() {
    printf("Max threads: %d\n", omp_get_max_threads());
    //set the number of threads
    omp_set_num_threads(4);

    #pragma omp parallel
    {
        //omp_get_thread_num() -> gets the id of threads that run in this region.
        printf("Hello, from thread %d\n", omp_get_thread_num());
    }
    return 0;
}