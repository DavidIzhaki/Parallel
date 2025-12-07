#include <omp.h>
#include <stdio.h>

int main() {
    int A[500][1000] = {0};
    double start, end;
    double a_time, b_time;
    
    start = omp_get_wtime();
    #pragma omp parallel for 
    for (int i = 0; i < 500; i++) {
        for (int j = 0; j < 1000; j++) {
            A[i][j] = i + j;
            A[i][j] = A[i][j]*A[i][j];
        }
    }
    end = omp_get_wtime();
    a_time = end - start;

    start = omp_get_wtime();
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < 500; i++) {
        for (int j = 0; j < 1000; j++) {
            A[i][j] = i + j;
            A[i][j] = A[i][j]*A[i][j];
        }
    }
    end = omp_get_wtime();
    b_time = end - start;



    #pragma omp parallel for collapse(n)
    for (.....) {
        for (....) {
            ......
        }
    }
    printf("time without collapse: %f\n",a_time);
    printf("time with collapse: %f\n",b_time);
    return 0;





}