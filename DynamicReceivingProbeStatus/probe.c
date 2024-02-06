#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv) {
    /*
    in the previous example we had to create a really large buffer for what was potentially a small amount of data
    we can use MPI_Probe to figure out exactly how large we need to make our buffer to avoid this
    MPI_Probe(
        int source, int tag, MPI_Comm comm, MPI_Status* status
    )
    this works like a receive without receiving the message but we can use it to fill the
    status struct which we can then use to receive the correct amount of data
    */

    MPI_Init(NULL, NULL);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    if (world_size != 2) {
        fprintf(stderr, "Must use two processes.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int number_amount;
    if (world_rank == 0) {
        const int MAX_NUMBERS = 100;
        int numbers[MAX_NUMBERS];

        srand(time(NULL));
        number_amount = (rand() / (float) RAND_MAX) * MAX_NUMBERS;

        MPI_Send(numbers, number_amount, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("0 sent %d numbers to 1\n", number_amount);
    } else if (world_rank == 1) {
        MPI_Status status;

        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        // now we can use the info in status to figure out how many things were sent

        MPI_Get_count(&status, MPI_INT, &number_amount);

        int* number_buffer = (int*) malloc(sizeof(int) * number_amount);

        MPI_Recv(number_buffer, number_amount, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("1 received %d items from 0", number_amount);

        // DO NOT FORGET TO FREE
        free(number_buffer);
    }

    MPI_Finalize();
}