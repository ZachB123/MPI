#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
    
    // mpi environment is initialized usually with *argc char*** argv three *?
    // communicator formed around all spawned processes
    MPI_Init(NULL, NULL);

    int world_size;
    // Comm_size returns the size of the communicator
    // MPI_COMM_WORLD encloses all of the processes
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    // gets the rank for this actual process, ranks start from 0
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    // gets the name of the processor that this program is executing on
    MPI_Get_processor_name(processor_name, &name_len);

    printf("Hello world from processor %s rank %d out of %d processors\n", 
        processor_name, world_rank, world_size);
    
    // no more mpi calls can be made after this
    MPI_Finalize();

}

// to artificially