#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>


// volatile int i = 0;
// char hostname[256];
// gethostname(hostname, sizeof(hostname));
// printf("PID %d on %s ready for attach\n", getpid(), hostname);
// fflush(stdout);
// while (0 == i)
//     sleep(5);


// this is how we will keep track of the processes when we conduct our parallel sort
// currently the code can only handle sorting integers and floats
// int_comm_rank is the rank of the processer that this data belongs to
// number is the actual data sent from the processor
typedef struct {
    int comm_rank;
    union {
        float f;
        int i;
    } number;
} CommRankNumber;

// int compare_ints(const void *a, const void *b) {
//     // we have pointers that are void because this is what qsort wants
//     // cast them to CommRankNumber pointers then derefererence to subtract normally
//     // I think we can use the integer because of how the floats are ordered and can be compared nicely but the tutorial has a seperate one for floats
//     return (((CommRankNumber*)a)->number.i - ((CommRankNumber*)b)->number.i);
// }

// THIS IS REALLY BAD YOU CAN'T SUBTRACT FLOATS LIKE THIS TO COMPARE 
// just return an int the verbose way
// int compare_floats(const void *a, const void *b) {
//     return (((CommRankNumber*)a)->number.f - ((CommRankNumber*)b)->number.f);
// }

// A comparison function for sorting float CommRankNumber values
int compare_float_comm_rank_number(const void *a, const void *b) {
  CommRankNumber *comm_rank_number_a = (CommRankNumber *)a;
  CommRankNumber *comm_rank_number_b = (CommRankNumber *)b;
  if (comm_rank_number_a->number.f < comm_rank_number_b->number.f) {
    return -1;
  } else if (comm_rank_number_a->number.f > comm_rank_number_b->number.f) {
    return 1;
  } else {
    return 0;
  }
}

// A comparison function for sorting int CommRankNumber values
int compare_int_comm_rank_number(const void *a, const void *b) {
  CommRankNumber *comm_rank_number_a = (CommRankNumber *)a;
  CommRankNumber *comm_rank_number_b = (CommRankNumber *)b;
  if (comm_rank_number_a->number.i < comm_rank_number_b->number.i) {
    return -1;
  } else if (comm_rank_number_a->number.i > comm_rank_number_b->number.i) {
    return 1;
  } else {
    return 0;
  }
}

void* gather_numbers_to_root(void* number, MPI_Datatype datatype, MPI_Comm comm) {
    int comm_rank, comm_size;
    MPI_Comm_rank(comm, &comm_rank);
    MPI_Comm_size(comm, &comm_size);

    int datatype_size;
    // assigns datatype_size to number of bytes that each datatype takes
    MPI_Type_size(datatype, &datatype_size);
    void* gathered_numbers;
    if (comm_rank == 0) {
        // there is a spot for one datatype from each process in the communication
        // remember the first number is from process 0 second from process 1 etc.
        gathered_numbers = malloc(datatype_size * comm_size);
    }

    // remember recv_count is number received per process
    MPI_Gather(number, 1, datatype, gathered_numbers, 1, datatype, 0, comm);

    // returns NULL uneless you are root
    // root must free this
    return gathered_numbers;
}

void print_comm_rank(CommRankNumber* comm_rank_number) {
    printf("comm_rank: %d, number: %f\n", comm_rank_number->comm_rank, comm_rank_number->number.f);
}

/*
This is run by the root and takes in the numbers gathered by gather_numbers to root
it returns the ranks that each process will hold i.e first value in return is rank of first process
*/
int* get_ranks(void* gathered_numbers, int gathered_number_count, MPI_Datatype datatype) {
    // steps
    // turn into structs
    // sort structs
    // turn into array
    // return

    int datatype_size;
    MPI_Type_size(datatype, &datatype_size);

    CommRankNumber* comm_ranks = (CommRankNumber*) malloc(gathered_number_count * sizeof(CommRankNumber));

    for (int i = 0; i < gathered_number_count; i++) {
        comm_ranks[i].comm_rank = i;
        memcpy(&(comm_ranks[i].number), gathered_numbers + (i * datatype_size), datatype_size);
    }

    // printf("Before sorting\n");
    // CommRankNumber* loc = comm_ranks;
    // for (int i = 0; i < 4; i++) {
    //     print_comm_rank(loc);
    //     loc++;
    // }

    if (datatype == MPI_INT) {
        qsort(comm_ranks, gathered_number_count, sizeof(CommRankNumber), &compare_int_comm_rank_number); // come back to see if & is needed
    } else if (datatype == MPI_FLOAT) {
        qsort(comm_ranks, gathered_number_count, sizeof(CommRankNumber), &compare_float_comm_rank_number);
    }

    // printf("After sorting\n");
    // loc = comm_ranks;
    // for (int i = 0; i < 4; i++) {
    //     print_comm_rank(loc);
    //     loc++;
    // }

    int* parallel_ranks = (int*) malloc(gathered_number_count * sizeof(int));

    for (int i = 0; i < gathered_number_count; i++) {
        parallel_ranks[comm_ranks[i].comm_rank] = i;
    }

    free(comm_ranks);
    return parallel_ranks;
}

/*
    Returns the parallel rank
    send_data takes in one number of datatype
*/
int TMPI_Rank(void* send_data, void* recv_data, MPI_Datatype datatype, MPI_Comm comm) {


    if (datatype != MPI_INT && datatype != MPI_FLOAT) {
        return MPI_ERR_TYPE;
    }

    int comm_size, comm_rank;
    MPI_Comm_size(comm, &comm_size);
    MPI_Comm_rank(comm, &comm_rank);


    void* gathered_nums = gather_numbers_to_root(send_data, datatype, comm);


    int* ranks = NULL;
    if (comm_rank == 0) {
        // printf("Here are the gathered numbers\n");
        // int datatype_size;
        // MPI_Type_size(datatype, &datatype_size);
        // float* loc = gathered_nums;
        // for (int i = 0; i < comm_size; i++) {
        //     printf("%f, ", *(float*)loc);
        //     loc += 1;
        // }
        // printf("\n");
        ranks = get_ranks(gathered_nums, comm_size, datatype);
    }

    MPI_Scatter(ranks, 1, datatype, recv_data, 1, datatype, 0, comm);

    if (comm_rank == 0) {
        free(gathered_nums);
        free(ranks);
    }

}