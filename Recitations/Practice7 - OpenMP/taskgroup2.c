#include <stdio.h>
#include <omp.h>

int main(void) {
    omp_set_num_threads(4);  
    #pragma omp parallel
    #pragma omp single nowait
    {
        #pragma omp taskgroup      // start a task group
        {
            #pragma omp task       // parent task in the group
            {
                #pragma omp task   // child task in the group
                {
                    printf("Hello.\n");
                }

                printf("Hi.\n");
            }
            // leaving taskgroup: wait for parent AND its descendants
        }

        printf("Goodbye.\n");
    }
    return 0;
}
