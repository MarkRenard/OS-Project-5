// logging.c was created by Mark Renard on 4/13/2020.
//
// This file contains definitions for functions that aid in the collection,
// formatting, and logging of data pertinent to Assignment 5.

#include "clock.h"
#include "constants.h"
#include "perrorExit.h"
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
	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "Master has detected Process P%d requesting %d of R%d at" \
		" time %03d : %09d\n", simPid, count, resourceId, 
		time.seconds, time.nanoseconds);

}

// Logs allocation of a resource
void logAllocation(int simPid, int resourceId, int count, Clock time){
	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "Master granted P%d reqest for %d of R%d at time " \
		" %03d : %09d\n", simPid, count, resourceId, time.seconds, 
		time.nanoseconds);

}

// Logs the id and quantity of resources being released at a particular time
void logResourceRelease(int simPid, int resourceId, int count, Clock time){
	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "Master has acknowledged Process P%d releasing %d of R%d" \
		" at time %03d : %09d\n", simPid, count, resourceId, 
		time.seconds, time.nanoseconds);

}

/*
void logSystemResources(int * allocation, pid_t pidArray){
	int i, j;	

	// Prints a row of column headers
	fprintf(log, "   ")
	for (i = 0; i < NUM_RESOURCES; i++){
		fprintf(log, "R%d ", i);
	}
	fprintf(log, "\n");
	lines++;

	// Prints resources allocated for each process
	for (i = 0; i < MAX_RUNNING; i++){

}
*/

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

// Prints the resource class ids and quantity of released resources
void logRelease(int * resourceIds, int * numReleased, int size){
	if (++lines > MAX_LOG_LINES || size < 1) return;

	// Prints message, first resource released and quantity
	fprintf(log, "\t\tResources released are as follows: R%d:%d",
		resourceIds[0], numReleased[0]);

	// Prints subsequent resources released and quantity
	int i = 0;
	for ( ; i < size; i++){
		fprintf(log, ", R%d:%d", resourceIds[i], numReleased[i]);
	}
	fprintf(log, "\n");
}


