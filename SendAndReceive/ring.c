/*
my attempt to write an MPI program
the goal is to have n processes where the 0 process starts with pi and passes 
it around to all the other processes until a complete circle is made

*/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int* argc, char** argv) {


    MPI_Init(NULL, NULL);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_size < 2) {
        fprintf(stderr, "World size must be at least 2 but was actually %d", world_size);
    }

    int next_rank = (world_rank + 1) % world_size;
    int prev_rank = (world_rank + (world_size - 1)) % world_size;

    float current_value = 0;
    if (world_rank == 0) {
        current_value = 3.14159;
    }
    
    // this is kinda wonky, in the tutorial all processes other than root receive then the send is unified and root receives at the end
    // it also looks like a negative source number also works ok
    // the receiving process must ackowledge that it wants to receive the data before the sender can continue

    if (world_rank == 0) {
        MPI_Send(&current_value, 1, MPI_FLOAT, next_rank, 0, MPI_COMM_WORLD);
        printf("Process %d sent %f to process %d\n", world_rank, current_value, next_rank);
        current_value = 0;
    }

    MPI_Recv(&current_value, 1, MPI_FLOAT, prev_rank, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Process %d received %f from process %d\n", world_rank, current_value, prev_rank);

    if (world_rank != 0) {
        MPI_Send(&current_value, 1, MPI_FLOAT, next_rank, 0, MPI_COMM_WORLD);
        printf("Process %d sent %f to process %d\n", world_rank, current_value, next_rank);
    }

    MPI_Finalize();
}
