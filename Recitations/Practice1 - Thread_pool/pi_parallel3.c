#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include "thr_pool.h"

#define TOTAL_SAMPLES      1000000LL
#define MIN_NUM_POOL_THREADS   4
#define MAX_NUM_POOL_THREADS   8
#define CHUNK_SIZE         1000LL

typedef struct {
    long long samples;
    long long *slot_hits;   /* private slot for this task */
    unsigned int seed;
} task_arg_t;

static void *calc_task(void *arg) {
    task_arg_t *a = (task_arg_t *)arg;
    long long local_hits = 0;

    for (long long i = 0; i < a->samples; i++) {
        double x = (double)rand_r(&a->seed) / (double)RAND_MAX;
        double y = (double)rand_r(&a->seed) / (double)RAND_MAX;

        if (x * x + y * y <= 1.0) {
            local_hits++;
        }
    }

    *(a->slot_hits) = local_hits;
    return NULL;
}

int main(int argc, char **argv) {
    long long total_samples = TOTAL_SAMPLES;
    if (argc > 1) {
        total_samples = atoll(argv[1]);
    }

    if (total_samples <= 0) {
        fprintf(stderr, "total_samples must be positive\n");
        return 1;
    }

    long long num_tasks = (total_samples + CHUNK_SIZE - 1) / CHUNK_SIZE;

    thr_pool_t *pool = thr_pool_create(
        MIN_NUM_POOL_THREADS,   /* min_threads */
        MAX_NUM_POOL_THREADS,   /* max_threads */
        5,                  /* linger seconds */
        NULL                /* default pthread attributes */
    );

    if (pool == NULL) {
        perror("thr_pool_create");
        return 1;
    }

    task_arg_t *args = malloc((size_t)num_tasks * sizeof(task_arg_t));
    long long *hits_array = calloc((size_t)num_tasks, sizeof(long long));

    if (args == NULL || hits_array == NULL) {
        perror("malloc/calloc");
        free(args);
        free(hits_array);
        thr_pool_destroy(pool);
        return 1;
    }

    for (long long i = 0; i < num_tasks; i++) {
        long long start = i * CHUNK_SIZE;
        long long remaining = total_samples - start;

        args[i].samples = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : remaining;
        args[i].slot_hits = &hits_array[i];
        args[i].seed = (unsigned int)time(NULL) ^ (unsigned int)(i * 1234567u);

        if (thr_pool_queue(pool, calc_task, &args[i]) != 0) {
            perror("thr_pool_queue");
            free(args);
            free(hits_array);
            thr_pool_destroy(pool);
            return 1;
        }
    }

    thr_pool_wait(pool);

    long long global_hits = 0;
    for (long long i = 0; i < num_tasks; i++) {
        global_hits += hits_array[i];
    }

    double pi = 4.0 * (double)global_hits / (double)total_samples;

    printf("Total samples = %lld\n", total_samples);
    printf("Chunk size    = %lld\n", CHUNK_SIZE);
    printf("Num tasks     = %lld\n", num_tasks);
    printf("Hits          = %lld\n", global_hits);
    printf("Estimated pi  = %.10f\n", pi);

    free(args);
    free(hits_array);
    thr_pool_destroy(pool);
    return 0;
}