#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int* argc, char** argv) {
    /*
    sending variable length data
    previously we ignored the status of the receive with MPI_STATUS_IGNORE
    we can use an MPI_Status struct to get more information sucha s
    MPI_SOURCE - rank of sender
    MPI_TAG - the tag of the message
    use function MPI_Get_count to get length of message
    MPI_Get_count(
        MPI_Status* status,
        MPI_Datatype datatype,
        int* count
    )
    MPI_Recv can take in MPI_ANY_SOURCE for rank and MPI_ANY_TAG for tag
    then we can use this status to get what we need
    */

    MPI_Init(NULL, NULL);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    if (world_size != 2) {
        fprintf(stderr, "Must use two processes for this example\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    const int MAX_NUMBERS = 100;
    int numbers[MAX_NUMBERS]; // array of 100
    int number_amount;

    if (world_rank == 0) {
        // time null gives time since the epoch
        srand(time(NULL)); // seed the random number generator
        // rand() gives an integer between 0 and RAND_MAX - defined by the library
        number_amount = (rand() / (float) RAND_MAX) * MAX_NUMBERS; // 0 - 99 inclusive
        
        // numbers is already a pointer since it is an array
        // we dont actually know how many things were sent just that the max is 100
        MPI_Send(numbers, number_amount, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("0 sent %d numbers to 1\n",  number_amount);
    } else if (world_rank == 1) {
        // this will hold the status of the receive
        MPI_Status status;

        // we receiving at most MAX_NUMBERS
        MPI_Recv(numbers, MAX_NUMBERS, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        // here we are getting the actual amount that was sent
        MPI_Get_count(&status, MPI_INT, &number_amount);

        printf("1 received %d numbers from 0. Message source = %d, tag = %d\n", number_amount, status.MPI_SOURCE, status.MPI_TAG);
    }

    // barrier is a synchronization point for all processes is world, basically if a process reaches here it will
    // wait until all other processes in the communication group reach here as well
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
}