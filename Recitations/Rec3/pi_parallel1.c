// pi_threadpool_slots.c — Monte Carlo π with thread-pool,
// per-thread result array (no malloc, no mutex, no atomics).
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include "thr_pool.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ---- timing (C11) ----
static inline double timespec_diff_sec(struct timespec a, struct timespec b) {
    long sec  = b.tv_sec  - a.tv_sec;
    long nsec = b.tv_nsec - a.tv_nsec;
    return (double)sec + (double)nsec * 1e-9;
}

// ---- RNG per task ----
static inline double urand01(unsigned int *seed) {
    return (double)rand_r(seed) / (RAND_MAX + 1.0);  // uniform [0,1)
}

// ---- task argument (lives in an array in main, not malloc'd) ----
typedef struct {
    uint64_t iters;     // samples for this thread
    uint64_t *slots;    // base of result array
    int       index;    // my slot index
    unsigned  seed;     // per-thread RNG seed
} task_arg_t;

// ---- worker ----
static void *calc_task(void *vp) {
    task_arg_t *a = (task_arg_t *)vp;

    uint64_t local_inside = 0;
    unsigned seed = a->seed;

    for (uint64_t i = 0; i < a->iters; ++i) {
        double x = urand01(&seed);
        double y = urand01(&seed);
        if (x*x + y*y <= 1.0) local_inside++;
    }

    a->slots[a->index] = local_inside;   // write to my private slot
    return NULL;
}

int main(int argc, char **argv) {
    // Total samples (default 10M) and fixed thread-count (default 8 as you asked)
    uint64_t N = (argc > 1) ? strtoull(argv[1], NULL, 10) : 10000000ULL;
    int min_max_threads = 8;
   

    printf("Samples: %llu, Threads: %d\n",
           (unsigned long long)N, min_max_threads);

    // Fixed-size pool: min = max = min_max_threads
    thr_pool_t *pool = thr_pool_create(min_max_threads, min_max_threads, /*linger=*/5, NULL);
    if (!pool) { perror("thr_pool_create"); return 1; }

    // Split work evenly, distribute remainder
    uint64_t base = N / (uint64_t)min_max_threads;
    uint64_t rem  = N % (uint64_t)min_max_threads;

    // Per-thread slots + args (stack arrays → no malloc)
    uint64_t  slots[/*VLA*/ (size_t)min_max_threads];
    task_arg_t args[/*VLA*/ (size_t)min_max_threads];

    // Init slots to 0
    for (int i = 0; i < min_max_threads; ++i) slots[i] = 0;

    struct timespec t0, t1;
    timespec_get(&t0, TIME_UTC);

    // Queue exactly one task per thread index
    for (int i = 0; i < min_max_threads; ++i) {
        args[i].iters = base + (i < (int)rem ? 1 : 0);
        args[i].slots = slots;
        args[i].index = i;
        args[i].seed  = (unsigned)time(NULL) ^ (unsigned)(0x9e3779b9u * (i + 1));

        if (thr_pool_queue(pool, calc_task, &args[i]) != 0) {
            perror("thr_pool_queue");
            // If queue fails, record zero for this slot
            slots[i] = 0;
        }
    }

    thr_pool_wait(pool);
    timespec_get(&t1, TIME_UTC);
    thr_pool_destroy(pool);

    // Sum all thread-local slots
    uint64_t inside_total = 0;
    for (int i = 0; i < min_max_threads; ++i) inside_total += slots[i];

    double elapsed = timespec_diff_sec(t0, t1);
    double pi_est  = 4.0 * (double)inside_total / (double)N;
    double err     = fabs(pi_est - M_PI);
    double thrpt   = (elapsed > 0.0) ? (double)N / elapsed : 0.0;

    printf("Inside:  %llu\n", (unsigned long long)inside_total);
    printf("pi ≈ %.10f (abs err %.10f)\n", pi_est, err);
    printf("Time: %.3f ms (%.6f s)\n", elapsed * 1e3, elapsed);
    printf("Throughput: %.2f samples/s\n", thrpt);
    return 0;
}
