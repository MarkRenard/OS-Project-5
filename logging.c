// logging.c was created by Mark Renard on 4/13/2020.
//
// This file contains definitions for functions that aid in the collection,
// formatting, and logging of data pertinent to Assignment 5.

#include "clock.h"
#include "constants.h"
#include "perrorExit.h"
#include "matrixRepresentation.h"
#include "resourceDescriptor.h"
#include <stdio.h>

static FILE * log = NULL;
static int lines = 0;

// Opens the log file with name LOG_FILE_NAME or exits with an error message
void openLogFile(){
	if ((log = fopen(LOG_FILE_NAME, "w+")) == NULL)
		perrorExit("logging.c - failed to open log file");
}

// Logs the detection of a resource request
void logRequestDetection(int simPid, int resourceId, int count, Clock time){
#ifdef VERBOSE
	if (++lines > MAX_LOG_LINES) return;


	fprintf(log, "Master has detected Process P%d requesting %d of R%d at" \
		" time %03d : %09d\n", simPid, count, resourceId, 
		time.seconds, time.nanoseconds);
#endif
}

// Logs allocation of a resource
void logAllocation(int simPid, int resourceId, int count, Clock time){
#ifdef VERBOSE
	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "Master granted P%d request for %d of R%d at time " \
		" %03d : %09d\n", simPid, count, resourceId, time.seconds, 
		time.nanoseconds);
#endif
}

// Logs when a request is denied and placed in a queue for a resource
void logEnqueue(int simPid, int quantity, int rNum, int available){
#ifdef VERBOSE
	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "\tP%d requested %d of R%d but only %d available, " \
		"enqueueing request\n", simPid, quantity, rNum, available);
#endif
}

// Prints the resource allocation table every 20 requests by default
void logTable(ResourceDescriptor * resources){
#ifdef VERBOSE
	if (++lines > MAX_LOG_LINES) return;
	static int callCount = 0;	// Times called since last print

	// Restricts printing to once every 20 request by default
	callCount++;
	if (callCount < ALLOC_PER_TABLE) return;
	callCount = 0;

	// Prints the table	
	int m, n;	// m resources, n processes
	
	// Prints header
	fprintf(log, "\n     ");
	for (m = 0; m < NUM_RESOURCES; m++){
		fprintf(log, "R%02d ", m);
	}
	fprintf(log, "\n");
	lines++;

	// Prints rows
	for (n = 0; n < MAX_RUNNING; n++){
		fprintf(log, "P%02d: ", n);

		// Prints each resource
		for (m = 0; m < NUM_RESOURCES; m++)
			fprintf(log, "%02d  ", resources[m].allocations[n]);

		fprintf(log, "\n");
		lines++;
	}

	fprintf(log, "\n");
	lines++;
#endif
}

// Logs the id and quantity of resources being released at a particular time
void logResourceRelease(int simPid, int resourceId, int count, Clock time){
#ifdef VERBOSE
	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "Master has acknowledged Process P%d releasing %d of R%d" \
		" at time %03d : %09d\n", simPid, count, resourceId, 
		time.seconds, time.nanoseconds);
#endif
}

// Prints a line that deadlock detection is being run
void logDeadlockDetection(Clock time){
	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "Master running deadlock detection at time %03d : %09d:" \
		"\n", time.seconds, time.nanoseconds);

}

// Prints the pids of processes in deadlock
void logDeadlockedProcesses(int * deadlockedPids, int size){
	if (++lines > MAX_LOG_LINES || size < 1) return;

	fprintf(log, "\tProcesses P%d", deadlockedPids[0]);

	int i = 1;
	for ( ; i < size; i++){
		fprintf(log, ", P%d", deadlockedPids[i]);
	}

	fprintf(log, " deadlocked\n");
}

// Prints that a deadlock resolution attempt is being made
void logResolutionAttempt(){
	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "\tAttempting to resolve deadlock...\n");
}

// Prints a message indicating that a process with logical pid was killed
void logKill(int simPid){
	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "\tKilling process P%d\n", simPid);
}

// Prints a message indicating that deadlock has been resolved
void logResolutionSuccess(){
	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "\tSystem is not in deadlock\n");
}

// Prints a message indicating that a process has terminated on its own
void logCompletion(int simPid){
#ifdef VERBOSE
	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "\tMaster has responded to P%d completing\n", simPid);
#endif
}

// Prints the resource class ids and quantity of released resources
void logRelease(int * resources){
	int released[NUM_RESOURCES];	// Number of each resource reached
	int indices[NUM_RESOURCES];	// Index of each resource

	// Copies non-zero allocations & indices to released, returns if zero
	int i, j = 0;
	for (i = 0; i < NUM_RESOURCES; i++){
		if (resources[i] != 0){
			released[j] = resources[i];
			indices[j] = i;
			j++;
		}
	}

	// Returns if no resources released
	if (j == 0) return;

	// Returns if max log lines reached	
	if (++lines > MAX_LOG_LINES) return;

	// Prints message, first resource released and quantity
	fprintf(log, "\t\tResources released are as follows: R%d:%d",
		indices[0], released[0]);

	// Prints subsequent resources released and quantity
	for (i = 1; i < j; i++){
		fprintf(log, ", R%d:%d", indices[i], released[i]);
	}
	fprintf(log, "\n");
}

// Prints table of m resources, n processes
int printTable(FILE * fp, const int * table, int m, int n){
	// Prints the table	
	int r, p;	// resource and process indices
	int lines = 0;	// Number of lines added
	
	// Prints header of resources
	fprintf(fp, "\t     ");
	for (r = 0; r < m; r++){
		fprintf(fp, "R%02d ", r);
	}
	fprintf(fp, "\n");
	lines++;

	// Prints rows of table
	for (p = 0; p < n; p++){
		fprintf(fp, "\tP%02d: ", p);

		// Prints each resource
		for (r = 0; r < m; r++)
			fprintf(fp, "%02d  ", table[p*m + r]);

		fprintf(fp, "\n");
		lines++;
	}
	fprintf(fp, "\n");
	lines++;

	return lines;
}

// Prints allocated, requested, and available matrices to a file, returns num \n
int printMatrices(FILE * fp, const int * allocated, const int * request, 
		  const int * available){

	// Rare instance where not having a named constant is probably best
	int addedLines = 6;
	
        fprintf(fp, "\n\tAllocation matrix:\n");
        addedLines += printTable(fp, allocated, NUM_RESOURCES, MAX_RUNNING);

        fprintf(fp, "\n\tRequest matrix:\n");
        addedLines += printTable(fp, request, NUM_RESOURCES, MAX_RUNNING);

        fprintf(fp, "\n\tAvailable vector:\n");
	addedLines += printTable(fp, available, NUM_RESOURCES, 1);

	return addedLines;
}

// Prints a matrix representation of the state of the program to a file
int printMatrixRep(FILE * fp, const ResourceDescriptor * resources){
        int allocated[NUM_RESOURCES * MAX_RUNNING];     // Resource allocation
        int request[NUM_RESOURCES * MAX_RUNNING];       // Current requests
        int available[NUM_RESOURCES];                   // Available resources

        // Initializes vectors
        setAllocated(resources, allocated);
        setRequest(resources, request);
        setAvailable(resources, available);

	// Prints matrices
	return printMatrices(fp, allocated, request, available);
}

// Logs a matrix representation of the system state
void logMatrixRep(const ResourceDescriptor * resources){
	if (lines > MAX_LOG_LINES) return;

	lines += printMatrixRep(log, resources);
}

// Logs allocated, requested, and available matrices
void logMatrices(const int * allocated, const int * request, 
		 const int * available){
	if (lines > MAX_LOG_LINES) return;

	lines += printMatrices(log, allocated, request, available);

}
