#include <stdio.h>
#include <omp.h>

int main(void) {
    int sum = 1;

    #pragma omp parallel for firstprivate(sum) lastprivate(sum)
    for (int i = 0; i < 4; i++) {
        sum += i;
        printf("i=%d ID%d sum=%d\n", i, omp_get_thread_num(), sum);
    }

    printf("--------------\n");
    printf("sum=%d\n", sum);

    return 0;
}
