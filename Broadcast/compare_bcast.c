#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>


// SLOW BROADCAST
void my_bcast(void* data, int count, MPI_Datatype datatype, int root,MPI_Comm communicator) {
    int world_rank;
    MPI_Comm_rank(communicator, &world_rank);
    int world_size;
    MPI_Comm_size(communicator, &world_size);

    if (world_rank == root) {
        // If we are the root process, send our data to everyone
        int i;
        for (i = 0; i < world_size; i++) {
            if (i != world_rank) {
            MPI_Send(data, count, datatype, i, 0, communicator);
            }
        }
    } else {
        // If we are a receiver process, receive the data from the root
        MPI_Recv(data, count, datatype, root, 0, communicator, MPI_STATUS_IGNORE);
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: compare_bcast num_elements num_trials\n");
    }

    int num_elements = atoi(argv[1]);
    int num_trials = atoi(argv[2]);

    MPI_Init(NULL, NULL);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    double total_my_bcast_time = 0.0;
    double total_mpi_bcast_time = 0.0;
    int* data = (int*) malloc(sizeof(int)* num_elements);

    // this makes sure we actually got a pointer to data i.e enough memory in the heap
    assert(data != NULL);

    for (int i = 0; i < num_trials; i++) {
        // MPI_Barrier doesnt let any process pass until all processes in the comm group make it here
        MPI_Barrier(MPI_COMM_WORLD);
        // MPI_Wtime retuns a float representing the number of seconds since a time in the past
        total_my_bcast_time -= MPI_Wtime();
        my_bcast(data, num_elements, MPI_INT, 0, MPI_COMM_WORLD);

        // synchronize all processes after the broadcast
        MPI_Barrier(MPI_COMM_WORLD);
        total_my_bcast_time += MPI_Wtime();

        MPI_Barrier(MPI_COMM_WORLD);
        total_mpi_bcast_time -= MPI_Wtime();
        MPI_Bcast(data, num_elements, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        total_mpi_bcast_time += MPI_Wtime();
    }


    // note to self, I think the speedup of MPI_Bcast isn't really shown here because I am not actually running
    // the processes on multiple cpus.
    if (world_rank == 0) {
    printf("Data size (bytes) = %d, Trials = %d\n", num_elements * (int)sizeof(int),
            num_trials);
    printf("Avg my_bcast time = %lf\n", total_my_bcast_time / num_trials);
    printf("Avg MPI_Bcast time = %lf\n", total_mpi_bcast_time / num_trials);
    }


    // IF YOU MALLOC DO NOT FORGET TO FREE THIS IS VERY IMPORTANT ZACH
    free(data);

    MPI_Finalize();
}