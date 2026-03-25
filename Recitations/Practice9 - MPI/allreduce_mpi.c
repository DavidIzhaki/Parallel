// monte_carlo_allreduce.c
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

    long long local_iters = 1000000;        // per-rank iterations
    if (argc >= 2) local_iters = atoll(argv[1]);

    // unique seed per rank
    unsigned seed = (unsigned)time(NULL) ^ (unsigned)getpid() ^ (unsigned)(rank * 0x9e3779b9u);
    srand(seed);

    long long local_inside = 0;
    for (long long i = 0; i < local_iters; i++) {
        double x = urand01();
        double y = urand01();
        if (x*x + y*y <= 1.0) local_inside++;
    }

    long long total_inside = 0;
    MPI_Allreduce(&local_inside, &total_inside, 1,
                  MPI_LONG_LONG, MPI_SUM,
                  MPI_COMM_WORLD);

    long long total_iters = local_iters * (long long)size;
    double pi = 4.0 * (double)total_inside / (double)total_iters;

    printf("Rank %d: pi ~= %.10f (total iters=%lld)\n", rank, pi, total_iters);

    MPI_Finalize();
    return 0;
}
