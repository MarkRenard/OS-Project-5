// resourceDescriptor.h was created by Mark Renard on 4/11/2020.
//
// This file contains the definition of a ResourceDescriptor, which records
// allocation information for instances of a simulated resource.

#ifndef RESOURCEDESCRIPTOR_H
#define RESOURCEDESCRIPTOR_H

#include "constants.h"
#include "queue.h"
#include <stdbool.h>

typedef struct resourceDescriptor{
	bool shareable;			// Whether the resource is shareable
	int numInstances;		// Total number of existing instances
	int numAvailable;		// Number of unclaimed instances
	int allocations[MAX_RUNNING];	// Number owned by each logical pid

	Queue waiting;			// Queue of pending requests
} ResourceDescriptor;

void initResources(ResourceDescriptor *);

#endif
