#include <stdio.h>
#include <stdlib.h>
#include "thr_pool.h"

// Function that will be executed by the thread pool
void *print_number(void *arg) {
    int num = *(int *)arg;
    printf("Number: %d\n", num);
    free(arg); // Free the memory allocated for the number
    return NULL;
}

int main() {
     // Pool configuration
    int min_threads = 2;
    int max_threads = 4;
    int linger_sec  = 5;
    printf("Max threads: %d\n", max_threads);

    // Print the max threads first (as requested)
    printf("Max threads: %d\n", max_threads);

    // Create the pool
    thr_pool_t *pool = thr_pool_create(min_threads, max_threads, linger_sec, NULL);
    if (!pool) {
        perror("thr_pool_create failed");
        return 1;
    }


    // Queue 4 jobs to print numbers 1 to 4
    for (int i = 1; i <= 4; i++) {
        int *num = malloc(sizeof(int)); // Allocate memory for the number
        if (num == NULL) {
            perror("Failed to allocate memory");
            continue;
        }
        *num = i;
        thr_pool_queue(pool, print_number, num); // Add the job to the pool
    }

    thr_pool_wait(pool);      // Wait until all jobs are done
    thr_pool_destroy(pool);   // Destroy the thread pool and free resources

    return 0;
}
