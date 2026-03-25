#include <stdio.h>
#include <omp.h>

int main(void) {
    int sum = 1;
    omp_set_num_threads(8);

    #pragma omp parallel firstprivate(sum)
    {
        sum += 5;
        printf("ID%d sum=%d\n", omp_get_thread_num(), sum);
    }

    printf("--------------\n");
    printf("sum=%d\n", sum);

    return 0;
}
