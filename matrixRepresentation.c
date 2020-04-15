// matrixRepresentation.c was created by Mark Renard on 4/15/2020.
//
// These functions return matrix representations of the state of the system.

#include "resourceDescriptor.h"
#include "constants.h"

// Sets the allocation matrix
void setAllocated(ResourceDescriptor * resources,
                          int * allocated){
        int m = NUM_RESOURCES;

        int r, p;
        for (p = 0; p < MAX_RUNNING; p++) {
                for (r = 0; r < NUM_RESOURCES; r++){
                        allocated[p*m + r] = resources[r].allocations[p];
                }
        }
}

// Sets the request matrix
void setRequest(const ResourceDescriptor * resources,
                        int * request){
        int i, r, p;
        int m = NUM_RESOURCES;
        int n = MAX_RUNNING;

        Message * msg;

        // Initializes values to 0
        for (i = 0; i < m * n; i++){
                request[i] = 0;
        }

        // Sums requests in waiting queue of each resource descriptor
        for (r = 0; r < NUM_RESOURCES; r++){
                msg = resources[r].waiting.front;

                while (msg != NULL){
                        p = msg->simPid;
                        request[p*m + r] += msg->quantity;
                        msg = msg->previous;
                }
        }

}

// Sets the available vector
void setAvailable(ResourceDescriptor * resources,
                         int * available) {
        int r;
        for (r = 0; r < NUM_RESOURCES; r++){
                available[r] = resources[r].numAvailable;
        }
}

