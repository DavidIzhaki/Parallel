#include <mpi.h>
#include <stdio.h>

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int x;
    if (rank == 0) {
        x = 123;  // root sets the value
        printf("Rank 0: broadcasting x=%d to %d processes\n", x, size);
    } else {
        x = -1;   // other ranks start with a dummy value
    }

    // Everyone calls the same collective
    MPI_Bcast(&x, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // After broadcast, all ranks have x=123
    printf("Rank %d: after Bcast, x=%d\n", rank, x);

    MPI_Finalize();
    return 0;
}
