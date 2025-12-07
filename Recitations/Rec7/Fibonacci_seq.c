#include <stdio.h>

#define N 15

int main(void) {
    int fib_numbers[N];

    fib_numbers[0] = 1;
    fib_numbers[1] = 1;

    for (int i = 2; i < N; i++) {
        fib_numbers[i] = fib_numbers[i-1] + fib_numbers[i-2];
    }

    printf("The Fibonacci numbers are:\n");
    for (int i = 0; i < N; i++) {
        printf(" %d", fib_numbers[i]);
    }
    printf("\n");

    return 0;
}
