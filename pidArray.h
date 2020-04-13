// pidArray.h was created by Mark Renard on 4/12/2020.
//
// This file contains headers for functions that manipulate an array of pids.

#ifndef PIDARRAY_H
#define PIDARRAY_H

// Assigns a value of EMPTY to all elements in a pid_t array
void initPidArray(pid_t * pidArray);

// Returns the next logical pid corresponding to the index of an EMPTY value
int getLogicalPid(const pid_t * pidArray);

// Assigns EMPTY to the value in pidArray at the index with value pid
void removePid(pid_t * pidArray, pid_t pid);

#endif
