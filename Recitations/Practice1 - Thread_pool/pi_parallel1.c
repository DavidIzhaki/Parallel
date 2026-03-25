#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include "thr_pool.h"


typedef struct {
    long long samples;
    long long *global_hits;
    pthread_mutex_t *lock;
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

    pthread_mutex_lock(a->lock);
    *a->global_hits += local_hits;
    pthread_mutex_unlock(a->lock);

    return NULL;
}


#define TOTAL_SAMPLES 1000000LL
#define NUM_TASKS 4

int main(int argc, char **argv) {
    long long total_samples = TOTAL_SAMPLES;
    if (argc > 1) {
        total_samples = atoll(argv[1]);
    }

    long long global_hits = 0;
    pthread_mutex_t lock;
    pthread_mutex_init(&lock, NULL);

    thr_pool_t *pool = thr_pool_create(
        4,                  /* min_threads */
        NUM_TASKS,          /* max_threads */
        4,                  /* linger seconds */
        NULL                /* default pthread attributes */
    );

    if (pool == NULL) {
        perror("thr_pool_create");
        pthread_mutex_destroy(&lock);
        return 1;
    }

    task_arg_t *args = malloc(NUM_TASKS * sizeof(task_arg_t));
    if (args == NULL) {
        perror("malloc");
        thr_pool_destroy(pool);
        pthread_mutex_destroy(&lock);
        return 1;
    }

    long long base = total_samples / NUM_TASKS;
    long long rem  = total_samples % NUM_TASKS;

    for (int i = 0; i < NUM_TASKS; i++) {
        args[i].samples = base + (i < rem ? 1 : 0);
        args[i].global_hits = &global_hits;
        args[i].lock = &lock;
        args[i].seed = (unsigned int)time(NULL) ^ (unsigned int)(i * 1234567u);
        //0 means success, non-zero means failure
        if (thr_pool_queue(pool, calc_task, &args[i]) != 0) {
            perror("thr_pool_queue");
            free(args);
            thr_pool_destroy(pool);
            pthread_mutex_destroy(&lock);
            return 1;
        }
    }

    thr_pool_wait(pool);

    double pi = 4.0 * (double)global_hits / (double)total_samples;

    printf("Total samples = %lld\n", total_samples);
    printf("Hits          = %lld\n", global_hits);
    printf("Estimated pi  = %.10f\n", pi);

    thr_pool_destroy(pool);
    pthread_mutex_destroy(&lock);
    free(args);

    return 0;
}