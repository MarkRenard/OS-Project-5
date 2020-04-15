// resourceDescriptor.c was created by Mark Renard on 4/14/2020.
//
// This file contains the defintion of a function that initializes an array
// of resource descriptors.

#include "resourceDescriptor.h"
#include "randomGen.h"
#include "constants.h"

void initResources(ResourceDescriptor * descriptors){
	int i, j;
	for(i = 0; i < NUM_RESOURCES; i++){
		descriptors[i].shareable = randBinary(SHAREABLE_PROBABILITY);
		descriptors[i].numInstances = randInt(MIN_INST, MAX_INST);
		descriptors[i].numAvailable = descriptors[i].numInstances;
		
		for(j = 0; j < MAX_LAUNCHED; j++)
			descriptors[i].allocations[j] = 0;

	}

	// Prints to stderr
	for (i = 0; i < NUM_RESOURCES; i++){
		fprintf(stderr, "R%d - share: %d numInstances: %d "\
			"numAvailable: %d\n",
			i,
			descriptors[i].shareable,
			descriptors[i].numInstances,
			descriptors[i].numAvailable);
	}
}
