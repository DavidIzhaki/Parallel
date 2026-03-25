// demo2_isend_irecv.c
#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int send = (rank == 0) ? 111 : 222;
    int recv = -1;

    int peer = (rank == 0) ? 1 : 0;

    MPI_Request reqs[2];
    MPI_Irecv(&recv, 1, MPI_INT, peer, 0, MPI_COMM_WORLD, &reqs[0]);
    MPI_Isend(&send, 1, MPI_INT, peer, 0, MPI_COMM_WORLD, &reqs[1]);

    // Work overlap
    for (volatile int i = 0; i < 150000000; i++) {}

    MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);

    printf("Rank %d: sent %d, received %d\n", rank, send, recv);

    MPI_Finalize();
    return 0;
}
