#include <stdio.h>
#include <omp.h>

int main(void) {
    omp_set_num_threads(4);  
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task           // parent task
            {
                #pragma omp task       // child task
                {
                    printf("Hello.\n");
                }

                printf("Hi.\n");       // still in parent task
            }

            #pragma omp taskwait       // waits for the parent task only
            printf("Goodbye.\n");
        }
    }
    return 0;
}
