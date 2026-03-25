// reduce_min.c
#include <mpi.h>
#include <stdio.h>

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int local = rank;          // each rank contributes its rank
    int total = 0;             // only meaningful on root

    MPI_Reduce(&local, &total, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Sum of ranks 0..%d = %d\n", size - 1, total);
    }

    MPI_Finalize();
    return 0;
}
