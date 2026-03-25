// monte_mpi_bad_seed.c
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

    
    srand((unsigned)time(NULL));

    long long inside = 0;
    for (long long i = 0; i < iters; i++) {
        double x = urand01();
        double y = urand01();
        if (x * x + y * y <= 1.0) inside++;
    }

    // prints look the same across ranks (or extremely similar)
    printf("Rank %d: inside=%lld / %lld\n", rank, inside, iters);


    MPI_Finalize();
    return 0;
}
