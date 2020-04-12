// getSharedMemoryPointers.h was created by Mark Renard on 3/27/2020 and
// modified on 4/11/2020.
//
// This file contains a header for the function getSharedMemoryPointers to be
// used in assignment 4.

#ifndef GETSHAREDMEMORYPOINTERS_H
#define GETSHAREDMEMORYPOINTERS_H

#include "constants.h"
#include "message.h"
#include "protectedClock.h"
#include "resourceDescriptor.h"
#include "sharedMemory.h"

void getSharedMemoryPointers(char ** shm,  ProtectedClock ** systemClock,
                             ResourceDescriptor ** resources,
			     Message ** messages, int flags);

#endif
