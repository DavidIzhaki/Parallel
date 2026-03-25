#include <stdio.h>
#include <omp.h>

#define NITERS 100000

/* Dummy work functions just for the example */
float big_job(int i) {
    return (float)i * 0.001f;
}

float consume(float x) {
    return x * x;   // pretend this is some expensive operation
}

int main(void) {
    float res = 0.0f;

    #pragma omp parallel
    {
        float B;
        int i, id, nthrds;

        id     = omp_get_thread_num();
        nthrds = omp_get_num_threads();

        for (i = id; i < NITERS; i += nthrds) {
            B = big_job(i);

            #pragma omp critical
            {
                res += consume(B);
            }
        }
    }

    printf("Result = %f\n", res);
    return 0;
}
