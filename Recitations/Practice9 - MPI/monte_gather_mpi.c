// monte_carlo_gather.c
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>   // getpid()

static double urand01(void) {
    return rand() / (double)RAND_MAX;
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    long long local_iters = 1000000;          // per-rank iterations
    if (argc >= 2) local_iters = atoll(argv[1]);

    // Different seed per rank (important!)
    unsigned seed = (unsigned)time(NULL) ^ (unsigned)getpid() ^ (unsigned)(rank * 0x9e3779b9u);
    srand(seed);

    // Monte Carlo: count points inside quarter circle
    long long local_inside = 0;
    for (long long i = 0; i < local_iters; i++) {
        double x = urand01();
        double y = urand01();
        if (x * x + y * y <= 1.0) local_inside++;
    }

    // Gather all local_inside values on rank 0
    long long *all_inside = NULL;
    if (rank == 0) {
        all_inside = (long long*)malloc((size_t)size * sizeof(long long));
        if (!all_inside) {
            fprintf(stderr, "Rank 0: malloc failed\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    MPI_Gather(&local_inside, 1, MPI_LONG_LONG,
               all_inside,    1, MPI_LONG_LONG,
               0, MPI_COMM_WORLD);

    // Root sums and prints pi
    if (rank == 0) {
        long long total_inside = 0;
        for (int i = 0; i < size; i++) total_inside += all_inside[i];

        long long total_iters = local_iters * (long long)size;
        double pi = 4.0 * (double)total_inside / (double)total_iters;

        printf("Per-rank iters = %lld, processes = %d\n", local_iters, size);
        printf("Total iters    = %lld\n", total_iters);
        printf("pi ~= %.10f\n", pi);

        free(all_inside);
    }

    MPI_Finalize();
    return 0;
}
