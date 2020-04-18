// stats.c was created by Mark Renard on 4/18/2020.
//
// This file defines functions that log statistics related to the execution
// of oss in assignment 5.

#include "stats.h"

static Stats stats;
static long double percentageAcc = 0.0;

void initStats(){
        stats.numRequestsGranted = 0;
        stats.numProcessesKilled = 0;
        stats.numProcessesCompleted = 0;
        stats.numTimesDeadlockDetectionRun = 0;
        stats.numTimesDeadlocked = 0;

	stats.percentKilledPerDeadlock = -1.0;
}

// Records the number of times oss grants a request for resources
void statsRequestGranted(){
	stats.numRequestsGranted++;
}

// Records the number of times oss terminates a process
void statsProcessKilled(){
	stats.numProcessesKilled++;
}

// Records the number of times a process terminates successfully
void statsProcessCompleted(){
	stats.numProcessesCompleted++;
}

// Records the number of times deadlock detection is run
void statsDeadlockDetectionRun(){
	stats.numTimesDeadlockDetectionRun++;
}

// Records number of times deadlock detected and percentage of processes killed
void statsDeadlockResolved(int killed, int runningAtStart){
	percentageAcc += (double)killed/(double)runningAtStart;

	stats.numTimesDeadlocked++;
}

// Sets the percentage of running processes terminated per deadlock on average
static void setPercentKilled(Stats * st){
	st->percentKilledPerDeadlock = (double)percentageAcc \
		/ (double)st->numTimesDeadlocked * 100;
}

Stats getStats(){
	setPercentKilled(&stats);
	return stats;
}
