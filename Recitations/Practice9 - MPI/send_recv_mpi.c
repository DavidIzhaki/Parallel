#include <mpi.h>
#include <stdio.h>

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) {
            printf("Run with at least 2 processes (e.g., -np 2)\n");
        }
        MPI_Finalize();
        return 0;
    }

    const int tag = 0;

    if (rank == 0) {
        int x = 42;
        MPI_Send(&x, 1, MPI_INT, 1, tag, MPI_COMM_WORLD);
        printf("Rank 0 sent %d to rank 1\n", x);
    } else if (rank == 1) {
        int y = -1;
        MPI_Status status;

        MPI_Recv(&y, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
        printf("Rank 1 received %d from rank %d (tag=%d)\n",
               y, status.MPI_SOURCE, status.MPI_TAG);
    }

    MPI_Finalize();
    return 0;
}
