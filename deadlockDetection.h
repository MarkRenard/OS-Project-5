// deadlockDetection.h was created by Mark Renard on 4/14/2020.
//
// This file contains a header for a functon which repeatedly detects deadlock
// and attempts to resolve it by killing a process.

#ifndef DEADLOCKDETECTION_H
#define DEADLOCKDETECTION_H

#include "resourceDescriptor.h"
#include "message.h"

#include <sys/types.h>


int resolveDeadlock(pid_t * pidArray, volatile ResourceDescriptor * resources,
                    volatile Message * messages);

#endif
