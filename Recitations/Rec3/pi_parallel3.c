// pi_threadpool_tls.c — Monte Carlo π with thread-pool + per-worker TLS slots
// No pool changes required. No mutexes. No atomics on the hot path.
//
// Usage:
//   gcc -std=c11 -O3 -Wall -Wextra -pthread -o pi_tp_tls pi_threadpool_tls.c thr_pool.c -lm
//   ./pi_tp_tls [N [T [CHUNK]]]
//     N     = total samples (default 10,000,000)
//     T     = pool threads (min=max) (default 8)
//     CHUNK = samples per task (default 10,000; set 1 for one-point-per-task)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <stdatomic.h>
#include "thr_pool.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// -------- timing helpers (C11) --------
static inline double timespec_diff_sec(struct timespec a, struct timespec b) {
    long sec  = b.tv_sec  - a.tv_sec;
    long nsec = b.tv_nsec - a.tv_nsec;
    return (double)sec + (double)nsec * 1e-9;
}

// -------- RNG (per task) --------
static inline double urand01(unsigned int *seed) {
    // POSIX reentrant RNG; good enough for demo
    return (double)rand_r(seed) / (RAND_MAX + 1.0);
}

// -------- TLS for per-worker index (no pool changes) --------
// We lazily assign each *worker thread* a stable index 0..T-1
// the first time it runs one of our tasks.
static pthread_key_t   g_tls_key;
static pthread_once_t  g_tls_once   = PTHREAD_ONCE_INIT;
static atomic_int      g_next_index = 0;     // assigns 0,1,2,... as workers encounter tasks
static int             g_T          = 0;     // number of worker threads we created (min=max)
static void tls_key_init(void) {
    // store int* in TLS; free when thread exits
    pthread_key_create(&g_tls_key, free);
}
// Returns this worker's 0..T-1 index (assigns once if first time)
static inline int get_worker_index(void) {
    pthread_once(&g_tls_once, tls_key_init);
    void *p = pthread_getspecific(g_tls_key);
    if (p) return *(int*)p;

    // First time this worker runs a task → claim an index
    int *idx = (int*)malloc(sizeof *idx);
    if (!idx) return 0; // fallback
    int claim = atomic_fetch_add_explicit(&g_next_index, 1, memory_order_relaxed);
    // Defensive clamp: in our use T==min==max so claim < g_T holds
    if (g_T > 0 && claim >= g_T) claim = g_T - 1;
    *idx = claim;
    pthread_setspecific(g_tls_key, idx);
    return *idx;
}

// -------- task argument --------
typedef struct {
    uint64_t iters;    // samples in this micro-task
    uint64_t *slots;   // slots[g_T], one slot per worker
    unsigned  seed;    // per-task RNG seed
} task_arg_t;

// -------- worker task --------
static void *calc_task(void *vp) {
    task_arg_t *a = (task_arg_t *)vp;

    // do micro-batch locally
    uint64_t local_inside = 0;
    unsigned seed = a->seed;
    for (uint64_t i = 0; i < a->iters; ++i) {
        double x = urand01(&seed);
        double y = urand01(&seed);
        if (x*x + y*y <= 1.0) local_inside++;
    }

    // accumulate into this worker's private slot (no contention)
    int idx = get_worker_index();           // 0..g_T-1
    a->slots[idx] += local_inside;          // safe: same worker runs tasks serially
    return NULL;
}

int main(int argc, char **argv) {
    // Parse args
    uint64_t N = (argc > 1) ? strtoull(argv[1], NULL, 10) : 10000000ULL;  // total samples
    int      T = 16;                          // threads (min=max)
    uint64_t C = 1000000;     // samples per task
    if (T <= 0) T = 8;
    if (C == 0) C = 1;

    g_T = T; // publish to TLS helper

    // Derive number of tasks
    uint64_t ntasks = (N + C - 1) / C;

    printf("Samples: %llu  Threads: %d  Chunk: %llu  Tasks: %llu\n",
           (unsigned long long)N, T,
           (unsigned long long)C, (unsigned long long)ntasks);

    // Create fixed-size pool (min=max=T)
    thr_pool_t *pool = thr_pool_create(T, T, /*linger=*/5, NULL);
    if (!pool) { perror("thr_pool_create"); return 1; }

    // Allocate per-worker slots and per-task args (two allocations total)
    uint64_t  *slots = (uint64_t *)calloc((size_t)T, sizeof(uint64_t));
    task_arg_t *args = (task_arg_t *)malloc((size_t)ntasks * sizeof(task_arg_t));
    if (!slots || !args) {
        perror("alloc");
        free(slots); free(args);
        thr_pool_destroy(pool);
        return 1;
    }

    // Enqueue all micro-tasks
    struct timespec t0, t1;
    timespec_get(&t0, TIME_UTC);

    for (uint64_t k = 0; k < ntasks; ++k) {
        uint64_t start   = k * C;
        uint64_t remain  = (N > start) ? (N - start) : 0;
        uint64_t this_it = (remain < C) ? remain : C;

        args[k].iters = this_it;
        args[k].slots = slots;
        // simple decorrelated seed
        args[k].seed  = (unsigned)time(NULL) ^ (unsigned)(0x9e3779b9u * (k + 1));

        if (thr_pool_queue(pool, calc_task, &args[k]) != 0) {
            perror("thr_pool_queue");
            // if queueing fails, we just skip this micro-batch
        }
    }

    // Wait, time, destroy
    thr_pool_wait(pool);
    timespec_get(&t1, TIME_UTC);
    thr_pool_destroy(pool);

    // Reduce per-worker slots
    uint64_t inside_total = 0;
    for (int i = 0; i < T; ++i) inside_total += slots[i];

    free(args);
    free(slots);

    // Report
    double elapsed = timespec_diff_sec(t0, t1);
    double pi_est  = 4.0 * (double)inside_total / (double)N;
    double err     = fabs(pi_est - M_PI);
    double thrpt   = (elapsed > 0.0) ? (double)N / elapsed : 0.0;

    printf("Inside: %llu\n", (unsigned long long)inside_total);
    printf("pi ≈ %.10f (abs err %.10f)\n", pi_est, err);
    printf("Time: %.3f ms (%.6f s)\n", elapsed * 1e3, elapsed);
    printf("Throughput: %.2f samples/s\n", thrpt);
    return 0;
}
