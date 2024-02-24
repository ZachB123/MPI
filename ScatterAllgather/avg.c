#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/*

MPI_Scatter(
    void* send_data, // resides on root process, important data
    int send_count, // how much sent to each process the number of elements divided by number of processes if doesn't divide evenly it will be covered later
    MPI_Datatype send_datatype,
    void* recv_data, // where the receviving stuff lands
    int recv_count, // is there any reason this wouldn't be the same as send count
    MPI_Datatype recv_datatype,
    int root, // root process to send the data
    MPI_Comm communicator
)

this is an identical signature
MPI_Gather(
    void* send_data,
    int send_count,
    MPI_Datatype send_datatype,
    void* recv_data, ONLY ROOT NEEDS TO HAVE A VALID ONE, other processes can send in null
    int recv_count, total number received per process NOT TOTAL NUMBER OUT OF ALL OF THEM
    MPI_Datatype recv_datatype,
    int root,
    MPI_Comm communicator)

*/

int main(int* argc, char** argv) {

    // goal is to generate a random array of 100 numbers then split across 10 processes to calculate average
    const int DATA_LENGTH = 100;
    const int NUMBER_PROCESSES = 10;

    MPI_Init(NULL, NULL);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (world_size != NUMBER_PROCESSES) {
        // fprint allows you to write to a file stream so like a file or stderr
        fprintf(stderr, "World size must be %d was actually %d\n", NUMBER_PROCESSES, world_size);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int send_count = DATA_LENGTH / NUMBER_PROCESSES;
    int* buffer = (int*) malloc(send_count * sizeof(int));

    // YOU DON'T NEED TO HAVE THE SCATTERS AND GATHERS SPLIT ACROSS IF STATEMENTS
    if (world_rank == 0) {

        srand(time(NULL));

        int* data = (int*) malloc(DATA_LENGTH  * sizeof(int));

        for (int i = 0; i < DATA_LENGTH; i++) {
            data[i] = (rand() / (float) RAND_MAX) * 10;
        }

        printf("Root array contents.\n");

        for (int i = 1; i <= DATA_LENGTH; i++) {
            printf("%d", data[i - 1]);
            if (i % 10 == 0) {
                printf("\n");
            }
        }


        MPI_Scatter(data, send_count, MPI_INT, buffer, send_count, MPI_INT, 0, MPI_COMM_WORLD);

        free(data);

    } else {
        MPI_Scatter(NULL, 0, MPI_INT, buffer, send_count, MPI_INT, 0, MPI_COMM_WORLD);
    }

    // I think all the processes should have their buffer properly set

    char output[100];

    sprintf(output, "Process %d is going to print out array contents.\n", world_rank);
    for (int i = 0; i < send_count; i++) {
        char temp[2];
        sprintf(temp, "%d", buffer[i]);
        strcat(output, temp);
    }
    strcat(output, "\n");

    int partial_sum = 0;
    for (int i = 0; i < send_count; i++) {
        partial_sum += buffer[i];
    }
    float partial_average = partial_sum / (1.0 * send_count);

    printf("%sProcess %d has partial average of %.3f.\n", output, world_rank, partial_average);

    if (world_rank == 0) {

        float partials[NUMBER_PROCESSES];
        MPI_Gather(&partial_average, 1, MPI_FLOAT, &partials, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);

        float sum = 0.0;
        for (int i = 0; i < NUMBER_PROCESSES; i++) {
            sum += partials[i];
        }

        printf("Average is %.3f.\n", sum / NUMBER_PROCESSES);

    } else {
        MPI_Gather(&partial_average, 1, MPI_FLOAT, NULL, 10, MPI_FLOAT, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    
}