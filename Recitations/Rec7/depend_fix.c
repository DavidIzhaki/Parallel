#include <stdio.h>
#include <omp.h>

int main(void) {
    int number;

    #pragma omp parallel
    #pragma omp single nowait
    {
        // Task A: initialize number
        #pragma omp task depend(out: number)
        number = 1;

        // Task B: print and increment (depends on A)
        #pragma omp task depend(inout: number)
        {
            printf("Task 1: current value of number is %d\n", number);
            number++;
        }

        // Task C: final print (depends on B)
        #pragma omp task depend(in: number)
        printf("Task 2: value of number at the end is %d\n", number);
    }

    return 0;
}