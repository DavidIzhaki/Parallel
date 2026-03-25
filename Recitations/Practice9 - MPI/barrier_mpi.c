#include <mpi.h>
#include <stdio.h>
#include <unistd.h>   // sleep

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Different ranks "arrive" at different times
    sleep(rank);  // rank 0 sleeps 0s, rank 1 sleeps 1s, ...

    printf("Rank %d reached the barrier\n", rank);

    // Everyone waits here until ALL ranks arrive
    MPI_Barrier(MPI_COMM_WORLD);

    printf("Rank %d passed the barrier\n", rank);

    MPI_Finalize();
    return 0;
}
