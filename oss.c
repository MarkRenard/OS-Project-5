// oss.c was created by Mark Renard on 4/11/2020.
//
// This program simulates deadlock detection and resolution.

#include "clock.h"
#include "getSharedMemoryPointers.h"
#include "message.h"
#include "perrorExit.h"
#include "protectedClock.h"
#include "resourceDescriptor.h"

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

static void simulateResourceManagement(ProtectedClock *, ResourceDescriptor *,
				       Message *); 
static void assignSignalHandlers();
static void cleanUpAndExit(int param);
static void cleanUp();

static char * shm;	// Pointer to the shared memory region

int main(int argc, char * argv[]){
	ProtectedClock * systemClock;		// Shared memory system clock
	ResourceDescriptor * resources;		// Shared memory resource table
	Message * messages;			// Shared memory message vector

	exeName = argv[0];	// Assigns exeName for perrorExit
	assignSignalHandlers(); // Sets response to ctrl + C & alarm
	alarm(MAX_SECONDS);	// Limits total execution time

	srand(BASE_SEED - 1);   // Seeds pseudorandom number generator

	// Creates shared memory region and gets pointers
	getSharedMemoryPointers(&shm, &systemClock, &resources, 
				&messages, IPC_CREAT);

	// Generates processes, grants requests, and resolves deadlock in a loop
	simulateResourceManagement(systemClock, resources, messages);

	cleanUp();

	return 0;
}



// Generates processes, grants requests, and resolves deadlock in a loop
void simulateResourceManagement(ProtectedClock * systemClock,
			        ResourceDescriptor * resourcesDescriptors,
			        Message * messages){

	printf("simulateResourceManagement called!\n");
/*

	Clock timeToFork = zeroClock();			// 
	Clock timeToDetect = DETECTION_INTERVAL;

	pid_t pidArray[MAX_RUNNING];
	initPidArray(pidArray);
	int running = 0;
	int launched = 0;

	do {
		// Launches user processes at random times
		if (clockCompare(timeToFork, *systemClock) >= 0){
			 
			// Launches process and records real pid if within limit
			if (running <= MAX_RUNNING && launched <= MAX_LAUNCHED){
				int index = getAvailableLogicalPid(pidArray);
				pidArray[index] = launchUserProcess(index);
				running++;
			}

			// Selects new random time to launch a new user process
			incrementClock(&timeToFork, randomTime(MIN_FORK_TIME,
							       MAX_FORK_TIME));
		}

		// Checks message vector for request or release of resources
		processRequests(*systemClock, resourceDescriptors, messages);

		// Detects deadlock at regular intervals
		if (clockCompare(timeToDetect, *systemClock) >= 0)
			while (deadlockDetected(resourceDescriptors))
				killAProcess(pidArray, resourceDescriptors);

	} while (!pidArrayEmpty(pidArray));
*/
}


// Determines the processes response to ctrl + c or alarm
static void assignSignalHandlers(){
	struct sigaction sigact;

	// Initializes sigaction values
	sigact.sa_handler = cleanUpAndExit;
	sigact.sa_flags = 0;

	// Assigns signals to sigact
	if ((sigemptyset(&sigact.sa_mask) == -1)
	    ||(sigaction(SIGALRM, &sigact, NULL) == -1)
	    ||(sigaction(SIGINT, &sigact, NULL)  == -1)){

		// Prints error message and exits on failure
		char buff[BUFF_SZ];
		sprintf(buff, "%s: Error: Failed to install signal handlers",
			exeName);
		perror(buff);
		exit(1);
	}
}

// Signal handler - closes files, removes shm, terminates children, and exits
static void cleanUpAndExit(int param){

	// Closes files, removes shm, terminates children
	cleanUp();

	// Prints error message
	char buff[BUFF_SZ];
	sprintf(buff,
		 "%s: Error: Terminating after receiving a signal",
		 exeName
	);
	perror(buff);

	// Exits
	exit(1);
}

// Ignores interrupts, kills child processes, closes files, removes shared mem
static void cleanUp(){
	// Handles multiple interrupts by ignoring until exit
	signal(SIGALRM, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	// Kills all other processes in the same process group
	kill(0, SIGQUIT);

	// Detatches from and removes shared memory
	detach(shm);
	removeSegment();
}

