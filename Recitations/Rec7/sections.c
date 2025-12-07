#include <stdio.h>
#include <omp.h>

int main(void) {
    omp_set_num_threads(3);
    #pragma omp parallel 
    {
        #pragma omp  sections
        {
            #pragma omp section
            {
                printf("Task A on thread %d\n", omp_get_thread_num());
                /* e.g., read input data */
            }

            #pragma omp section
            {
                printf("Task B on thread %d\n", omp_get_thread_num());
                /* e.g., preprocess data */
            }

            #pragma omp section
            {
                printf("Task C on thread %d\n", omp_get_thread_num());
                /* e.g., write results to file */
            }
            // implicit barrier here: wait for all sections
        }
        printf("Hello from thread: %d\n", omp_get_thread_num());
    }   


    return 0;
}
