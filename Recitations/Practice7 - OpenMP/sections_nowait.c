#include <stdio.h>
#include <omp.h>

int main(void) {
omp_set_num_threads(3);
    #pragma omp parallel
    {
        #pragma omp sections nowait
        {
            #pragma omp section
            {
                printf("Loading data on thread %d\n", omp_get_thread_num());
                /* simulate work */
            }

            #pragma omp section
            {
                printf("Computing stats on thread %d\n", omp_get_thread_num());
                /* simulate work */
            }

            #pragma omp section
            {
                printf("Logging info on thread %d\n", omp_get_thread_num());
                /* simulate work */
            }
        }   // no barrier here because of 'nowait'

        // Threads that finish their section early can continue immediately:
        printf("Thread %d continues after sections\n", omp_get_thread_num());
    }

    return 0;
}
