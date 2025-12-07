#include <stdio.h>
#include <omp.h>

int main(void) {
    int number;

    #pragma omp parallel
    #pragma omp single nowait
    {
        // Task A: initialize
        #pragma omp task
        number = 1;

        // Task B: print and increment
        #pragma omp task
        {
            printf("Task 1: current value of number is %d\n", number);
            number++;
        }

        // Task C: final print
        #pragma omp task
        printf("Task 2: value of number at the end is %d\n", number);
    }

    return 0;
}
