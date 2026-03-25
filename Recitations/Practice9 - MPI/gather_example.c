#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int local = rank * 10;     // each rank has different value
    int *all = NULL;

    if (rank == 0)
        all = malloc(size * sizeof(int));

    MPI_Gather(&local, 1, MPI_INT,
            all,   1, MPI_INT,
            0, MPI_COMM_WORLD);

    if (rank == 0) {
        // all = [0, 10, 20, 30, ...]
        for (int i = 0; i < size; i++)
            printf("all[%d] = %d\n", i, all[i]);
        free(all);
    }


    MPI_Finalize();
    return 0;
}
