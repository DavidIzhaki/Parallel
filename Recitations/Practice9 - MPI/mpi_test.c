    // demo4_test.c
#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Request req;
    int flag = 0;
    int data = 0;

    if (rank == 0) {
        data = 42;
        MPI_Isend(&data, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &req);

        while (!flag) {
            // do some work
            for (volatile int i = 0; i < 50000000; i++) {}

            MPI_Test(&req, &flag, MPI_STATUS_IGNORE);
            printf("Rank 0: Test flag = %d\n", flag);
        }

        printf("Rank 0: send completed (via Test).\n");
    } else if (rank == 1) {
        // Deliberately delay posting receive (to see polling)
        for (volatile int i = 0; i < 300000000; i++) {}
        MPI_Recv(&data, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Rank 1: received %d\n", data);
    }

    MPI_Finalize();
    return 0;
}
