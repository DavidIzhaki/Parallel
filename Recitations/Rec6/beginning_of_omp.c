#include <stdio.h>
#include <omp.h>
#define N    64 
int main(void){
    //set the number of threads
    omp_set_num_threads(4);

    //Open a parallel region 
    #pragma omp parallel
    {
        //omp_get_thread_num() -> gets the id of threads that run in this region.
        printf("Hello, from thread %d\n", omp_get_thread_num());
    }
    
    
    #pragma omp parallel for schedule(kind, chunk_size)
    for (int i = 0; i < N; i++) {
        work(i);
    }
    
    return 0;
}