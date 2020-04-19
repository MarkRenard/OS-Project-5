// resourceDescriptor.c was created by Mark Renard on 4/14/2020.
//
// This file contains the defintion of a function that initializes an array
// of resource descriptors.

#include "resourceDescriptor.h"
#include "randomGen.h"
#include "constants.h"

// Initializes resource with random instances and a chance of being shareable
void initResources(ResourceDescriptor * descriptors){
	int i, j;
	for(i = 0; i < NUM_RESOURCES; i++){

		// Resource is shareable with some probability (.2 by default)
		descriptors[i].shareable = randBinary(SHAREABLE_PROBABILITY);

		// Resource has a random number of instances (default 1 to 10)
		descriptors[i].numInstances = randInt(MIN_INST, MAX_INST);

		// All instances begin as available
		descriptors[i].numAvailable = descriptors[i].numInstances;
		
		// Sets number allocated to each process to 0
		for(j = 0; j < MAX_RUNNING; j++)
			descriptors[i].allocations[j] = 0;

	}

}
