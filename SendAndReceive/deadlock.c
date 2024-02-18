#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int* argc, int** argv) {
    // the goal of this program is to create a deadlock 

    MPI_Init(NULL, NULL);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    srand(time(NULL) * (world_rank + 1));


    if (world_size < 2) {
        fprintf(stderr, "World size must be at least 2 but was actually %d", world_size);
    }

    int next_rank = (world_rank + 1) % world_size;
    int previous_rank = (world_rank + (world_size - 1)) % world_size;

    int my_number = (rand() / (float) RAND_MAX) * 10;
    int received_number;


    MPI_Send(&my_number, 1, MPI_INT, next_rank, 0, MPI_COMM_WORLD);
    printf("Process %d sent value %d to process %d.\n", world_rank, my_number, next_rank);

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Recv(&received_number, 1, MPI_INT, previous_rank, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Process %d received %d from process %d.\n", world_rank, received_number, previous_rank);

    MPI_Barrier(MPI_COMM_WORLD);
    printf("%d All processes finished\n", world_rank);

    MPI_Finalize();

}