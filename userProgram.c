// userProgram.c was created by Mark Renard on 4/12/2020
//
// This program sends messages to an operating system simulator, simulating a
// process that requests and relinquishes resources at random times.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "constants.h"
#include "getSharedMemoryPointers.h"
#include "perrorExit.h"
#include "protectedClock.h"
#include "sharedMemory.h"

const struct timespec SLEEP = {0, 1000};

char * shm;	// Shared memory region pointer

int main(int argc, char * argv[]){
        ProtectedClock * systemClock;           // Shared memory system clock
        ResourceDescriptor * resources;         // Shared memory resource table
        Message * messages;                     // Shared memory message vector

	exeName = argv[0];		// Sets exeName for perrorExit
	int simPid = atoi(argv[1]);	// Gets process's logical pid

	getSharedMemoryPointers(&shm, &systemClock, &resources, &messages, 0);

	srand(BASE_SEED + simPid); // getPTime(systemClock).nanoseconds);

	fprintf(stderr, "\n\tPROCESS P%d RUNNING!\n\n", simPid);

	nanosleep(&SLEEP, NULL);
	
	detach(shm);
	
	fprintf(stderr, "\n\tPROCESS P%d COMPLETING!\n\n", simPid);

	return 0;
}
