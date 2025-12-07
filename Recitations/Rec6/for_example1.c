#include <stdio.h>
#include <omp.h>

int main(void) {

    #pragma omp parallel
    {
        #pragma omp for 
        for (int i = 0; i < 4; i++) {
            printf("i=%d, ID=%d\n", i, omp_get_thread_num());
    }
    } 


    #pragma omp parallel for
    for (int i = 0; i < 4; i++) {
        printf("i=%d, ID=%d\n", i, omp_get_thread_num());
    }

    return 0;
}