// demo6_waitany.c
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

static void spin_delay(long long iters) {
    for (volatile long long i = 0; i < iters; i++) {}
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 3) {
        if (rank == 0) printf("Run with at least 3 processes.\n");
        MPI_Finalize();
        return 0;
    }

    if (rank == 0) {
        MPI_Request* reqs = malloc((size - 1) * sizeof(MPI_Request));
        MPI_Status status;
        int* recvbuf = malloc((size - 1) * sizeof(int));

        // Post Irecv from each sender (1..size-1)
        for (int p = 1; p < size; p++) {
            MPI_Irecv(&recvbuf[p - 1], 1, MPI_INT, p, 0, MPI_COMM_WORLD, &reqs[p - 1]);
        }

        int remaining = size - 1;
        while (remaining > 0) {
            int index;
            MPI_Waitany(size - 1, reqs, &index, &status);
            int sender_rank = status.MPI_SOURCE;
            int value = recvbuf[index];
            printf("Rank 0: got %d from rank %d (index=%d)\n", value, sender_rank, index);
            remaining--;
        }

        free(reqs);
        free(recvbuf);
    } else {
        // Different delays -> different arrival order
        spin_delay(150000000LL * (long long)(size - rank));
        int v = 10 * rank;
        MPI_Send(&v, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        printf("Rank %d: sent %d\n", rank, v);
    }

    MPI_Finalize();
    return 0;
}
