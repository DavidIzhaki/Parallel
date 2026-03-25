#include <stdio.h>
#include <omp.h>

int main(void) {
    int x = 0, y = 1;

    #pragma omp parallel for default(none) shared(x, y)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 3; j++) {
            x += i;
            y += j;
        }
    }

    printf("x = %d, y = %d\n", x, y);
    return 0;
}
