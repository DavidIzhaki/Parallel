// demo5_waitall.c
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (size < 2) {
        if (rank == 0) printf("Run with at least 2 processes.\n");
        MPI_Finalize();
        return 0;
    }
   
    if (rank == 0) {
        
        MPI_Request* reqs = malloc((size - 1) * sizeof(MPI_Request));
        int* bufs = malloc((size - 1) * sizeof(int));

        for (int p = 1; p < size; p++) {
            bufs[p - 1] = 1000 + p;
            MPI_Isend(&bufs[p - 1], 1, MPI_INT, p, 0, MPI_COMM_WORLD, &reqs[p - 1]);
        }

        MPI_Waitall(size - 1, reqs, MPI_STATUSES_IGNORE);
        printf("Rank 0: all Isend operations completed.\n");

        free(reqs);
        free(bufs);
    } else {
        int x;
        MPI_Recv(&x, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Rank %d: received %d\n", rank, x);
    }

    MPI_Finalize();
    return 0;
}
