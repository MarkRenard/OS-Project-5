// resolveDeadlock.c was created by Mark Renard on 4/14/2020.
//
// This file contains functions for detecting deadlock, including Dr. Sanjiv 
// Bhatia's implementation of the deadlock detection algorithm.

#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "clock.h"
#include "constants.h"
#include "logging.h"
#include "matrixRepresentation.h"
#include "message.h"
#include "oss.h"
#include "perrorExit.h"
#include "pidArray.h"
#include "qMsg.h"
#include "resourceDescriptor.h"

// True if all values in req are <= values in avail for one process
static bool req_lt_avail ( const int*req, const int*avail, const int pnum, \
                    const int num_res )
{
    int i = 0 ;
    for ( ; i < num_res; i++ )
        if ( req[pnum*num_res+i] > avail[i] ) 
            break;
    
    return ( i == num_res );
}

// Returns true if the system is in deadlock and logs deadlocked processes
static bool deadlock ( const int*available, const int m, const int n, \
                const int*request, const int*allocated , int * deadlocked) 
{
    int  work[m];       // m resources
    bool finish[n];     // n processes
   
    int i; 
    for ( i = 0 ; i < m; i++ )
	work[i] = available[i];

    for ( i = 0 ; i < n; finish[i++] = false );

#ifdef DEBUG    
    fprintf(stderr, "\n\nFinished sequence: <");
#endif
    int p = 0;
    for ( ; p < n; p++ )   // For each process
    {
        if ( finish[p] ) continue;
        if ( req_lt_avail ( request, work, p, m ) )
        {
            finish[p] = true;
#ifdef DEBUG
            fprintf(stderr, "p%d, ", p);
#endif
            for ( i = 0 ; i < m; i++ )
                work[i] += allocated[p*m+i];
            p = -1;
        }
        
    }
#ifdef DEBUG
    fprintf(stderr, ">\n");

    fprintf(stderr, "Deadlock with processes <");
#endif
    bool deadlock = false;
    int deadPids[MAX_RUNNING];
    i = 0;
    for ( p = 0; p < n; p++ )
        if ( ! finish[p] )
        {
#ifdef DEBUG 
           fprintf(stderr, "p%d, ", p);
#endif
            deadlocked[p] = 1;
            deadPids[i++] = p;
            deadlock = true;

        }

    // Prints deadlocked processes to the log file
    logDeadlockedProcesses(deadPids, i);
#ifdef DEBUG
    fprintf(stderr, ">\n");
#endif
    
    return ( deadlock );
}

// Sets all the values in a vector to n
static void initVector(int * vector, int size, int n){
	int i = 0;
	for ( ; i < size; i++){
		vector[i] = n;
	}
}

// Kills the process with the greatest request and removes it from the pidArray
static void killAProcess(pid_t * pidArray, int * deadlocked, 
			 Message * messages,
			 ResourceDescriptor * resources){
	int p;			// Process index variable
	int maxRequest = 0;	// Greatest quantity of a resource requested
	int greediest = -1;	// Index of the process with greatest request

	// Loops through all logical pids
	for (p = 0; p < MAX_RUNNING; p++){

		// Selects deadlocked processes
		if (deadlocked[p] && messages[p].quantity > maxRequest){
			maxRequest = messages[p].quantity;
			greediest = p;
		}
	}

	// Kills the process
	killProcess(greediest, pidArray[greediest]);
	printMatrixRep(stderr, resources);
	
	// Removes the pid from the array
	if (pidArray[greediest] == EMPTY) perrorExit("killAProcess - no pid");
	pidArray[greediest] = EMPTY;


	
}

// Converts values in resource descriptors and messages to matrix form
static void updateMatrices(const ResourceDescriptor * resources, 
			   int * allocated, int * request, int * available,
			   int * deadlocked){
	
	setAllocated(resources, allocated);
	setRequest(resources, request);
	setAvailable(resources, available);
	initVector(deadlocked, MAX_RUNNING, 0);
}

// Detects and resolves deadlock - returns num killed and removes pids
int resolveDeadlock(pid_t * pidArray, ResourceDescriptor * resources,
		    Message * messages){
#ifdef DEBUG
	fprintf(stderr, "Resolving Deadlock\n");
#endif
	int allocated[NUM_RESOURCES * MAX_RUNNING];	// Resource allocation
	int request[NUM_RESOURCES * MAX_RUNNING];	// Current requests
	int available[NUM_RESOURCES];			// Available resources

	int deadlocked[MAX_RUNNING];	// Whether each pid is deadlocked

	int killed = 0;			// The number of terminated processes

	// Initializes vectors
	updateMatrices(resources, allocated, request, available, deadlocked);
	
#ifdef DEBUG
	printMatrices(stderr, allocated, request, available);
#endif
	// If deadlock exists, repeatedly kills processes until resolved
	bool printed = false;
	while(deadlock(available, NUM_RESOURCES, MAX_RUNNING, request,
		       allocated, deadlocked)){

		// Prints resolution message once
		if (!printed){
			logResolutionAttempt();
			printed = true;
		}

#ifdef DEBUG
		fprintf(stderr, "\t\t\t\t\t!!!DEADLOCK DETECTED!!!\n");
#endif
		killAProcess(pidArray, deadlocked, messages, resources);
		killed++;

		// Updates vectors	
		updateMatrices(resources, allocated, request, available, deadlocked);
	}

	fprintf(stderr, "End of deadlock resolution. Killed: %d\n", killed);

	logResolutionSuccess();

	return killed;
}



