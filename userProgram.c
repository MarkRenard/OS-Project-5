// userProgram.c was created by Mark Renard on 4/12/2020
//
// This program sends messages to an operating system simulator, simulating a
// process that requests and relinquishes resources at random times.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#include "constants.h"
#include "getSharedMemoryPointers.h"
#include "perrorExit.h"
#include "protectedClock.h"
#include "randomGen.h"
#include "sharedMemory.h"

// Prototypes
static void requestResources(ResourceDescriptor *, Message *, int);
static void releaseResources(ResourceDescriptor *, Message *, int);
static int selectResourceIndex(const Message * msg, int simPid);

// Constants
static const struct timespec SLEEP = {0, 1000};
static const Clock MIN_INC = {0, 0};
static const Clock MAX_INC = {0, 250 * MILLION};

// Static global
static char * shm;	// Shared memory region pointer

int main(int argc, char * argv[]){
	exeName = argv[0];		// Sets exeName for perrorExit
	int simPid = atoi(argv[1]);	// Gets process's logical pid
	srand(BASE_SEED + simPid); 	// Seeds pseudorandom number generator

        ProtectedClock * systemClock;	// Shared memory system clock
        ResourceDescriptor * resources;	// Shared memory resource table
        Message * messages;		// Shared memory message vector

	Clock decisionTime;		// Time to request, relese, or terminate
	Clock startTime;		// Time the process started

	fprintf(stderr, "\n\tPROCESS P%d RUNNING!\n", simPid);

	int size = getSharedMemoryPointers(&shm, &systemClock, &resources, &messages, 0);
	printSharedMemory(shm, size);

	fprintf(stderr, "\tP%d SHM ADDRESS: %p\n", simPid, shm);
	fprintf(stderr, "\tP%d CLOCK: %p\n", simPid, systemClock);
	fprintf(stderr, "\tP%d RESOURCES: %p\n", simPid, resources);
	fprintf(stderr, "\tP%d MESSAGES ADDRESS: %p\n\n", simPid, messages);


	// Initializes clocks
	startTime = getPTime(systemClock);
	decisionTime = startTime;
	fprintf(stderr, "\n\tP%d DECISION TIME - %03d : %09d\n\n",
		simPid, decisionTime.seconds, decisionTime.nanoseconds);

	// Repeatedly requests or releases resources or terminates
	while (!(messages[simPid].type == TERMINATION)) {

		// Decides when current time is at or after decision time
		if (clockCompare(getPTime(systemClock), decisionTime) >= 0){

			// Updates decision time
			incrementClock(&decisionTime, 
					randomTime(MIN_INC, MAX_INC));

			// Decides whether to terminate
			if (randBinary(TERMINATION_PROBABILITY)){
				fprintf(stderr, "\n\tPROCESS %d TERMINATING\n\n",
					simPid);

				messages[simPid].type = TERMINATION;

			// Decides whether to request or release resources
			} else if (randBinary(REQUEST_PROBABILITY)){
				requestResources(resources, messages, simPid);
			} else {
				releaseResources(resources, messages, simPid);
			}	
		}

		// Waits for response to request
		while(messages[simPid].type != VOID);
	}

	// Prepares to exit
	detach(shm);
	fprintf(stderr, "\n\tPROCESS P%d COMPLETING!\n\n", simPid);

	return 0;
}

// Creates a request in shared memory for a random number of a random resource
static void requestResources(ResourceDescriptor * resources, Message * messages,
			     int simPid){

	// Randomly selects the resource descriptor of a resource to request
	int rNum = randInt(0, NUM_RESOURCES - 1);

	// Calculates maximum request quantity
	int allocated = resources[rNum].allocations[simPid];
	int maxRequest = resources[rNum].numInstances - allocated;

	// Makes the request if more can be requested
	if (maxRequest > 0){

		// Increments num classes if no resources of this class held
		if (messages[simPid].target[rNum] == 0)
			messages[simPid].numClassesHeld++;

		// Adds a requested number of instances to the target number
		int numRequested = randInt(1, maxRequest);
		messages[simPid].quantity = numRequested;
		messages[simPid].target[rNum] += numRequested;
		messages[simPid].rNum = rNum;

		fprintf(stderr, "\n\tPROCESS %d REQUESTING %d OF R%d\n",
			simPid, numRequested, rNum);

		messages[simPid].type = REQUEST;
		fprintf(stderr, "\tMSG[%d].TYPE: %d ADDRESS: %p\n\n", 
			simPid, messages[simPid].type, &(messages[simPid].type));
	}

}

// Sends message to release a random number of a random resource
static void releaseResources(ResourceDescriptor * resources, Message * messages,
			     int simPid){

	// Returns if no resources are held
	if (messages[simPid].numClassesHeld == 0) return;

	// Selects the index of a held resource randomly
	int rNum = selectResourceIndex(&messages[simPid], simPid);

	// Constructs message to release a random number of the chosen resource
	int numHeld = messages[simPid].target[rNum];
	int numReleased = randInt(1, numHeld);
	messages[simPid].quantity = numReleased;
	messages[simPid].target[rNum] -= numReleased;
	messages[simPid].rNum = rNum;

	// Decrements num classes if no resources of this class will be held
	if (messages[simPid].target[rNum] == 0)
		messages[simPid].numClassesHeld--;

	fprintf(stderr, "\n\tPROCESS %d RELEASING %d OF R%d\n\n",
		simPid, numReleased, rNum);

	messages[simPid].type = RELEASE;
}

// Randomly selects the index of a currently held resource in the request
static int selectResourceIndex(const Message * msg, int simPid){

	// Selects which held resource to release
	int classNum = randInt(1, msg->numClassesHeld);

	// Finds the rNum of that resource by checking held resource classes 
	int rNum = 0, checked = 0;
	for ( ; rNum < NUM_RESOURCES; rNum++){

		// Increments checked if resource at current rNum is held
		if (msg->target[rNum] != 0){
			checked++;		

			// Breaks if classNum reached
			if (checked == classNum) break;
		}
	}

	return rNum;
}
