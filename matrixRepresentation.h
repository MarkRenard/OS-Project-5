// matrixRepresentation.h was created by Mark Renard on 4/15/2020.
//
// This file contains headers for matrixRepresentation.c.

#ifndef MATRIXREPRESENTATION_H
#define MATRIXREPRESENTATION_H

#include "resourceDescriptor.h"

// Sets the allocation matrix
void setAllocated(const ResourceDescriptor * resources, int * allocated);

// Sets the request matrix
void setRequest(const ResourceDescriptor * resources, int * request);

// Sets the available vector
void setAvailable(const ResourceDescriptor * resources, int * available);

#endif
