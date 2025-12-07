#include <stdio.h>
#include <omp.h>

int main(void) {
    omp_set_num_threads(4);

    #pragma omp parallel
    {
        #pragma omp single
        {
            printf("Single thread %d creating tasks\n",
                   omp_get_thread_num());

            #pragma omp task
            {
                printf("Task A on thread %d\n", omp_get_thread_num());
            }
            #pragma omp taskwait
            printf("Task A finished, single thread %d continues\n",
                   omp_get_thread_num());

            #pragma omp task
            {
                printf("Task B on thread %d\n", omp_get_thread_num());
            }

            // Wait until A and B (and their children, if any) are done
           
        }
        
        
    }

    return 0;
}
