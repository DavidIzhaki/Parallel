#include <stdio.h>
#include <omp.h>


int main(void) {
    omp_set_num_threads(3);
    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            // The single thread creates tasks:
            #pragma omp task
            {
                printf("Task 1 on thread %d\n", omp_get_thread_num());
            }

            #pragma omp task
            {
                printf("Task 2 on thread %d\n", omp_get_thread_num());
            }

            #pragma omp task
            {
                printf("Task 3 on thread %d\n", omp_get_thread_num());
            }
            // implicit taskwait here at the end of 'single' block
        }
        printf("Hello from thread %d\n",  omp_get_thread_num());
    }

    return 0;
}
