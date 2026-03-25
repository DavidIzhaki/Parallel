// monte_mpi_good_seed.c
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>   // getpid()

static double urand01(void) {
    return rand() / (double)RAND_MAX;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    long long iters = 1000000;
    if (argc >= 2) iters = atoll(argv[1]);

    // FIX: mix rank into the seed so each process gets a different stream
    unsigned seed = (unsigned)time(NULL) ^ (unsigned)(rank * 0x9e3779b9u) ^ (unsigned)getpid();
    srand(seed);

    long long inside = 0;
    for (long long i = 0; i < iters; i++) {
        double x = urand01();
        double y = urand01();
        if (x * x + y * y <= 1.0) inside++;
    }

    printf("Rank %d: inside=%lld / %lld (seed=%u)\n", rank, inside, iters, seed);

    MPI_Finalize();
    return 0;
}
