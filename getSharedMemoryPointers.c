// getSharedMemoryPointers.c was created by Mark Renard on 4/11/2020.
//
// This file contains the definition of a shared memory function specific to
// assignment 5. This function is used by oss.c and userProgram.c.

#include "clock.h"
#include "constants.h"
#include "message.h"
#include "protectedClock.h"
#include "resourceDescriptor.h"
#include "sharedMemory.h"

int getSharedMemoryPointers(char ** shm,  ProtectedClock ** systemClock,
			     ResourceDescriptor ** resources,
			     Message ** messages, int flags) {

	// Computes size of the shared memory region
	int shmSize = sizeof(ProtectedClock) \
		      + sizeof(ResourceDescriptor) * NUM_RESOURCES \
                      + sizeof(Message) * MAX_RUNNING;

 	// Attaches to shared memory
        *shm = sharedMemory(shmSize, flags);

	// Gets pointer to simulated system clock
	*systemClock = (ProtectedClock *)(*shm);

	// Gets pointer to first resource descriptor
	*resources = (ResourceDescriptor *)(*shm + sizeof(ProtectedClock));

	// Gets pointer to message array
	*messages = (Message *)( ((char*)(*resources)) \
		     + (sizeof(ResourceDescriptor) * NUM_RESOURCES));

	return shmSize;
}

