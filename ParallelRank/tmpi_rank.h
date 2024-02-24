
// we check if the macros is created 
// __PARALLEL_RANK_H is just a 1 or 0 depending on if the macro has been created yet it prevents multiple inclusion errors
#ifndef __PARALLEL_RANK_H

#define __PARALLEL_RANK_H 1

// prefix MPI functions that we create with T to prevent namespace collisions
int TMPI_Rank(void *send_data, void *recv_data, MPI_Datatype datatype, MPI_Comm comm);

#endif