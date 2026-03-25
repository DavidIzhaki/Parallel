// demo1_isend.c
#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int data = 0;
    MPI_Request req;

    if (rank == 0) {
        data = 123;
        MPI_Isend(&data, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &req);

        // "Work" while message is in flight
        for (volatile int i = 0; i < 200000000; i++) {}

        // Must wait before reusing/modifying send buffer safely
        MPI_Wait(&req, MPI_STATUS_IGNORE);
        printf("Rank 0: Isend completed.\n");
    } else if (rank == 1) {
        MPI_Recv(&data, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Rank 1: received %d\n", data);
    }

    MPI_Finalize();
    return 0;
}
