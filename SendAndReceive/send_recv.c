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
    // argv[0] is the name of the file
    fprintf(stderr, "World size must be greater than 1 for %s\n", argv[0]);
    // 1 is the error code
    MPI_Abort(MPI_COMM_WORLD, 1); 
  }

  int number;
  if (world_rank == 0) {
    number = -1;
    /*
    signature for MPI_Send is as follows
    MPI_Send(
        void* data, a pointer to what we are sending to the other process
        int count, number of items in data
        MPI_Datatype datatype, the type of data MPI has a bunch of wrappers we can also define our own
        int destination, the rank of the process we are sending to
        int tag, these numbers can represent different messages and we can choose to only accept certain tags
        MPI_Comm communicator the communicator for our mpi setup
    )
    */
    // can also use tag MPI_ANY_TAG
    MPI_Send(&number, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
  } else if (world_rank == 1) {
    /*
    signature for MPI_Recv
    MPI_Recv(
        void* data, where the data from the sender will be stored
        int count, we will receive at most count elements
        MPI_Datatype datatype,
        int source, the source rank
        int tag,
        MPI_Comm communicator,
        MPI_Status* status information about received messages
    )
    */
    MPI_Recv(&number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Process 1 received number %d from process 0\n", number);
  }

  MPI_Finalize();
}