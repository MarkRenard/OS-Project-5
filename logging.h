// logging.h was created by Mark Renard on 4/13/2020.
//
// This file contains headers for functions that aid in the collection,
// formatting, and logging of data pertinent to Assignment 5.

#ifndef LOGGING_H
#define LOGGING_H

// Opens the log file with name LOG_FILE_NAME or exits with an error message
void openLogFile();

// Logs the detection of a resource request
void logRequestDetection(int simPid, int resourceId, int count, Clock time);

// Logs allocation of a resource
void logResourceAllocation(int simPid, int resourceId, int count, Clock time);

// Logs the ids and quantities of resources being released at a particular time
void logResourceRelease(int simPid, int resourceId, int count, Clock time);

// Prints a line that deadlock detection is being run
void logDeadlockDetection(Clock time);

// Prints the pids of processes in deadlock
void logDeadlockedProcesses(int * deadlockedPids, int size);

// Prints that a deadlock resolution attempt is being made
void logResolutionAttempt();

// Prints a message indicating that a process with logical pid was killed
void logKill(int simPid);

// Prints the resource class ids and count of released resources
void logRelease(int * resourceIds, int * numReleased, int size);

#endif
