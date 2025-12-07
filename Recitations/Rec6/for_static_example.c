#include <stdio.h>
#include <omp.h>

int main(void) {
    int N = 16;
    #pragma omp parallel for schedule(static, 2)
    for (int i = 0; i < N; i++) {
        printf("i = %2d, thread = %d\n", i, omp_get_thread_num());
    }

    return 0;
}