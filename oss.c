// oss.c was created by Mark Renard on 4/11/2020.
//
// This program simulates deadlock detection and resolution.

#include "clock.h"
#include "getSharedMemoryPointers.h"
#include "logging.h"
#include "message.h"
#include "perrorExit.h"
#include "pidArray.h"
#include "protectedClock.h"
#include "resourceDescriptor.h"

#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Constants
const Clock DETECTION_INTERVAL = {1, 0};
const Clock MIN_FORK_TIME = {0, 1 * MILLION};
const Clock MAX_FORK_TIME = {0, 250 * MILLION};
const Clock MAIN_LOOP_INCREMENT = {0, 50 * MILLION};

// Prototypes
static void simulateResourceManagement(ProtectedClock *, ResourceDescriptor *,
				       Message *);
static void populateDescriptors(ResourceDescriptor * resources);
static pid_t launchUserProcess(int simPid);
static bool processCompleted(pid_t * pidArray);
static void processMessages(ProtectedClock *, ResourceDescriptor *, Message *);
static bool deadlockDetected(Clock, const ResourceDescriptor *);
static bool resolveDeadlock(pid_t *, ResourceDescriptor *);
static void assignSignalHandlers();
static void cleanUpAndExit(int param);
static void cleanUp();

// Static global variables
static char * shm;	// Pointer to the shared memory region

int main(int argc, char * argv[]){
	ProtectedClock * systemClock;		// Shared memory system clock
	ResourceDescriptor * resources;		// Shared memory resource table
	Message * messages;			// Shared memory message vector

	exeName = argv[0];	// Assigns exeName for perrorExit
	assignSignalHandlers(); // Sets response to ctrl + C & alarm
//	alarm(MAX_SECONDS);	// Limits total execution time
	openLogFile();		// Opens file written to in logging.c

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
void simulateResourceManagement(ProtectedClock * clock,
			        ResourceDescriptor * resources,
			        Message * messages){

	Clock timeToFork = zeroClock();		 // Time to launch user process 
	Clock timeToDetect = DETECTION_INTERVAL; // Time to resolve deadlock

	pid_t pidArray[MAX_RUNNING];		// Array of user process pids
	int simPid;				// Temporary logical pid storage
	int running = 0;			// Currently running child count
	int launched = 0;			// Total children launched

	initPidArray(pidArray);			// Sets pids to -1
	initPClock(clock);			// Initializes protected clock
	populateDescriptors(resources);		// Sets initial resources

	int i = 0;

	// Launches processes and resolves deadlock until limits reached
	do {

		fprintf(stderr, "\nIteration %d:\n", i++);
		printPids(pidArray);

		// Waits for semaphore protecting system clock
		pthread_mutex_lock(&clock->sem);		

		// Launches user processes at random times
		if (clockCompare(clock->time, timeToFork) >= 0){
			 
			// Launches process & records real pid if within limits
			if (running < MAX_RUNNING && launched < MAX_LAUNCHED){
				simPid = getLogicalPid(pidArray);
				pidArray[simPid] = launchUserProcess(simPid);

				running++;
				launched++;
			}

			// Selects new random time to launch a new user process
			incrementClock(&timeToFork, randomTime(MIN_FORK_TIME,
							       MAX_FORK_TIME));
		}
		
		// Waits for completed child processes
		while (processCompleted(pidArray)) running--;

		// Checks for resource request or release, updates descriptors
		processMessages(clock, resources, messages);

		// Detects and resolves deadlock at regular intervals
		if (clockCompare(clock->time, timeToDetect) >= 0){
			if (deadlockDetected(clock->time, resources)){	
				do {
					running--;
				} while(!resolveDeadlock(pidArray, resources));
			}
		}

		incrementClock(&clock->time, MAIN_LOOP_INCREMENT);

		// Unlocks the system clock
		pthread_mutex_unlock(&clock->sem);

	} while (running > 0 || launched < MAX_LAUNCHED);
}

static void populateDescriptors(ResourceDescriptor * resources){
	printf("populateDescriptors called\n");
}

// Forks & execs a user process with the assigned logical pid, returns child pid
static pid_t launchUserProcess(int simPid){
	fprintf(stderr, "launchUserProcess(%d) called - ", simPid);

//	return 0;

	pid_t realPid;

	// Forks, exiting on error
	if ((realPid = fork()) == -1) perrorExit("Failed to fork");

	// Child process calls execl on the user program binary
	if (realPid == 0){
		char sPid[BUFF_SZ];
		sprintf(sPid, "%d", simPid);
		
		execl(USER_PROG_PATH, USER_PROG_PATH, sPid, NULL);
		perrorExit("Failed to execl");
	}

	fprintf(stderr, "Process %d real pid: %d\n", simPid, realPid);

	return realPid;

}

// Returns true of a process has completed and removes its pid from the array
static bool processCompleted(pid_t * pidArray){
//	int random;

	fprintf(stderr, "processCompleted called - ");
/*
	// Returns false if there are no "running processes"
	if (isEmpty(pidArray)) return false;

	// Returns false with probability 1/2
	if (rand() % 2) return false;
	
	// Removes a pid from the array and returns true
	random = randomPidIndex(pidArray);
	pidArray[random] = -1;

	fprintf(stderr, "processCompleted - pid %d \"completed\"\n", random);

	return true;
*/
	pid_t childPid;	// Index of a completed child

	// Waits for a completed child if one exists and gets child pid
	while((childPid = waitpid(-1, NULL, WNOHANG)) == -1 && errno == EINTR);

	// Returns false if no child has completed
	if (childPid == -1 || childPid == 0){
		fprintf(stderr, "no children completed\n");
		 return false;
	}

	fprintf(stderr, "removing real pid %d\n", childPid);
	
	// Sets value at index corresponding to the finished pid to EMPTY
	removePid(pidArray, childPid);

	return true;

}

// Reads message array, responds to requests, and releases resources
static void processMessages(ProtectedClock * clock, 
			    ResourceDescriptor * resources, Message * messages){
	fprintf(stderr, "processMessages called\n");

}

// Returns true if the current state is a deadlock state, logs time
static bool deadlockDetected(Clock now, const ResourceDescriptor * resources){
	bool detected = !((bool)(rand() % 5));

	fprintf(stderr, "deadlockDetected called - ");

	if (detected) fprintf(stderr, "DETECTED\n");
	else fprintf(stderr, "NOT detected\n");

	return detected;
}

// Selects a single process to terminate and returns true if deadlock resolved
static bool resolveDeadlock(pid_t * pidArray, ResourceDescriptor * resources){
	fprintf(stderr, "resolveDeadlock called - ");

	if (isEmpty(pidArray)) return true;

	int random = randomPidIndex(pidArray);
	pid_t randomPid = pidArray[random];

	kill(randomPid, SIGKILL);
	waitpid(randomPid, NULL, 0);

	fprintf(stderr, "removing process %d with pid %d\n", random, randomPid);

	removePid(pidArray, randomPid);

	return (bool)(rand() % 2);
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

