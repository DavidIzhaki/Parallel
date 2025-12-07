#include <stdio.h>
#include <omp.h>

#define N 15

int main(void) {
    int fib_numbers[N];

    #pragma omp parallel
    #pragma omp single
    {
        // Task for fib_numbers[0]
        #pragma omp task default(none) shared(fib_numbers) \
                         depend(out: fib_numbers[0])
        {
            fib_numbers[0] = 1;
        }

        // Task for fib_numbers[1]
        #pragma omp task default(none) shared(fib_numbers) \
                         depend(out: fib_numbers[1])
        {
            fib_numbers[1] = 1;
        }

        // Tasks for remaining Fibonacci numbers
        for (int i = 2; i < N; i++) {
            #pragma omp task default(none) shared(fib_numbers) firstprivate(i) \
                             depend(in:  fib_numbers[i-1], fib_numbers[i-2]) \
                             depend(out: fib_numbers[i])
            {
                fib_numbers[i] = fib_numbers[i-1] + fib_numbers[i-2];
            }
        }
        // End of single: implicit taskwait – all fib_numbers[] ready here
    }

    // Output the result
    printf("The Fibonacci numbers are:\n");
    for (int i = 0; i < N; i++) {
        printf(" %d", fib_numbers[i]);
    }
    printf("\n");

    return 0;
}
