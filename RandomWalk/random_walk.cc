
#include <iostream>
#include <vector>
#include <cstdlib>
#include <time.h>
#include <mpi.h>

/*
we have min max and a random walker W
W takes S random steps to the right
walker has a domain of max - min + 1, the + 1 since the max min values are inclusive
I think this is like the total number of spots the walker can be on
so we need to split this domain across all of our processes
we also want each process to initialize n walkers for whatever reason
*/

// this makes it so we don't have to preface stuff with std::
using namespace std;

typedef struct {
    int location;
    int steps_left; 
} Walker;

void decompose_domain(int domain_size, int world_rank, int world_size, int* subdomain_start, int* subdomain_size) {
    if (world_size > domain_size) {
        // world_size is the number of processes and so in this case there
        // would be more processes than spaces for walkers so we abort.
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // the starting location for this processor
    *subdomain_start = (domain_size / world_size) * world_rank;
    // how many locations of the walker holds
    *subdomain_size = domain_size / world_size;

    if (world_rank == world_size - 1) {
        // on the last processor we want to give any remainder
        *subdomain_size += domain_size % world_size;
    }
}

void initialize_walkers(int num_walkers, int max_walk_size, int subdomain_start, vector<Walker>* incoming_walkers) {
    Walker walker;
    // all walkers start at our subdomain start
    walker.location = subdomain_start;

    for (int i = 0; i < num_walkers; i++) {
        // give a random walk size
        walker.steps_left = (rand() / (float) RAND_MAX) * max_walk_size;
        // add walker to our walker list
        incoming_walkers->push_back(walker);
    }
}

void walk(Walker* walker, int subdomain_start, int subdomain_size, int domain_size, vector<Walker>* outgoing_walkers) {
    // do this until we finish the walk or we have to send the walker to another process
    while (walker->steps_left > 0) {
        if (walker->location == subdomain_start + subdomain_size) {
            if (walker->location == domain_size) {
                walker->location = 0;
            }
            outgoing_walkers->push_back(*walker);
            // done since it must be handled on the next process
            break;
        } else {
            walker->location++;
            walker->steps_left--;
        }
    }
}

void send_outgoing_walkers(vector<Walker>* outgoing_walkers, int world_rank, int world_size) {
    // we send the data as an array of bytes
    // data() returns a pointer to the underlying array of the vector
    // since we are sending in bytes we need the number of bytes in the underlying array which is the num elems times bytes per elem
    MPI_Send((void*) outgoing_walkers->data(), outgoing_walkers->size() * sizeof(Walker), MPI_BYTE, (world_rank + 1) % world_size, 0, MPI_COMM_WORLD);
    // clear outgoing walkers since we sent them
    outgoing_walkers->clear();
}

void receive_incoming_walkers(vector<Walker>* incoming_walkers, int world_rank, int world_size) {
    // I think these will always be used and if there are no walkers then the array will be empty

    // will use to get size of incoming walkers
    MPI_Status status;

    int incoming_rank = (world_rank == 0) ? world_size - 1 : world_rank - 1;
    
    MPI_Probe(incoming_rank, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    int incoming_size;

    MPI_Get_count(&status, MPI_BYTE, &incoming_size);

    incoming_walkers->resize(incoming_size / sizeof(Walker));

    MPI_Recv((void*) incoming_walkers->data(), incoming_size * sizeof(Walker), MPI_BYTE, incoming_rank, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

}

int main(int argc, char** argv) {

    int domain_size; // length of like walker route before it circles back
    int max_walk_size; // how far a walker can go
    int num_walkers_per_proc; // how many walkers each process creates

    if (argc < 4) {
        cerr << "Usage: random_walk domain_size max_walk_size "
                << "num_walkers_per_proc" << endl;
        exit(1);
    }

    domain_size = atoi(argv[1]);
    max_walk_size = atoi(argv[2]);
    num_walkers_per_proc = atoi(argv[3]);


    MPI_Init(NULL, NULL);

    // we must be very careful of deadlocks when we send our stuff out
    // we must guarantee that at least one send will immediately have a receive ready
    // MPI_Send will return when the send buffer can be reclaimed
    // this means that the data is on the network and we are free to use the buffer again

    // to solve deadlocks we will structure our programs so that even processes send and
    // odd processes receive, then the odd processes will send and even will receive

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // make sure the processes don't have the same random seed
    srand(time(NULL) * world_rank);

    int subdomain_start, subdomain_size;
    vector<Walker> incoming_walkers, outgoing_walkers;

    // now we now the length this processor holds and where it begins
    decompose_domain(domain_size, world_rank, world_size, &subdomain_start, &subdomain_size);

    // the walker vector we begin using is the incoming one
    initialize_walkers(num_walkers_per_proc, max_walk_size, subdomain_start, &incoming_walkers);

    cout << "Process " << world_rank << " initiated " << num_walkers_per_proc << " walkers in subdomain " << subdomain_start << " - " 
        << subdomain_start + subdomain_size - 1 << endl;

    // in this program each process can figure out when to terminate due to the max sends/receives to finish
    int maximum_sends_recv = max_walk_size / (domain_size / world_size) + 1;

    // now we have the giant loop that does everything
    // we first walk the walkers we have
    // then even processes send then receive
    // odd processes receive then send and we repeat
    for (int m = 0; m < maximum_sends_recv; m++) {
        
        // walking our walkers
        // the walk function only walks one walker
        for (int i = 0; i < incoming_walkers.size(); i++) {
            walk(&incoming_walkers[i], subdomain_start, subdomain_size, domain_size, &outgoing_walkers);
        }

        cout << "Process " << world_rank << " sending " << outgoing_walkers.size() << " walkers to process " << (world_rank + 1) % world_size << endl;

        if (world_rank % 2 == 0) {
            send_outgoing_walkers(&outgoing_walkers, world_rank, world_size);
            receive_incoming_walkers(&incoming_walkers, world_rank, world_size);
        } else {
            receive_incoming_walkers(&incoming_walkers, world_rank, world_size);
            send_outgoing_walkers(&outgoing_walkers, world_rank, world_size);
        }

        cout << "Process " << world_rank << " received " << incoming_walkers.size()
            << " incoming walkers" << endl;
    }
    cout << "Process " << world_rank << " done" << endl;

    MPI_Finalize();

    return 0;
}
