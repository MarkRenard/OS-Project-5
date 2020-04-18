// stats.h was created by Mark Renard on 4/18/2020.
//
// This file contains the definition of a struct with collected statistics on
// the execution of oss in assignment 5, along with functions for collecting them.

#ifndef STATS_H
#define STATS_H

typedef struct stats {
	unsigned long int numRequestsGranted;
	unsigned long int numProcessesKilled;
	unsigned long int numProcessesCompleted;
	unsigned long int numTimesDeadlockDetectionRun;
	unsigned long int numTimesDeadlocked;

	double percentKilledPerDeadlock;
} Stats;

void initStats();
void statsRequestGranted();
void statsProcessKilled();
void statsProcessCompleted();
void statsDeadlockDetectionRun();
void statsDeadlockResolved(int, int);
Stats getStats();


#endif
