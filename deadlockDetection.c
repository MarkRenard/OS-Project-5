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


    int p = 0;
    for ( ; p < n; p++ )   // For each process
    {
        if ( finish[p] ) continue;
        if ( req_lt_avail ( request, work, p, m ) )
        {
            finish[p] = true;

            for ( i = 0 ; i < m; i++ )
                work[i] += allocated[p*m+i];
            p = -1;
        }
        
    }

    bool deadlock = false;
    int deadPids[MAX_RUNNING];
    i = 0;
    for ( p = 0; p < n; p++ )
        if ( ! finish[p] )
        {

            deadlocked[p] = 1;
            deadPids[i++] = p;
            deadlock = true;

        }

    // Prints deadlocked processes to the log file
    logDeadlockedProcesses(deadPids, i);

    
    return ( deadlock );
}

// Sets all the values in a vector to n
static void initVector(int * vector, int size, int n){
	int i = 0;
	for ( ; i < size; i++){
		vector[i] = n;
	}
}

// Kills process with resources that meet a request or the greatest allocation
static void killAProcess(pid_t * pidArray, int * deadlocked, 
			 Message * messages,
			 ResourceDescriptor * resources){
	int killPid = -1;	// Logical pid of process to kill
	int maxAlloc = 0;	// Greatest num allocated of a needed resource
	int maxPid = -1;	// simPid of process with greatest allocation

	int quant;		// Quantity of requested resource
	int rNum;		// Index of requested resource
	int p, k;		// Index variables

	// Loops through all logical pids
	for (p = 0; p < MAX_RUNNING; p++){
		if (deadlocked[p]){

			// Gets request values
			rNum = messages[p].rNum;
			quant = messages[p].quantity;

			// Looks for deadlocked process that can meet request
			for (k = 0; k < MAX_RUNNING; k++){
			    if (deadlocked[k] && k != p){

				// Checks for new maximum allocation		
				if (resources[rNum].allocations[k] > maxAlloc){
				    maxAlloc = resources[rNum].allocations[k];
				    maxPid = k;
				}

				// Breaks if process k has enough
				if (resources[rNum].allocations[k] >= quant){
				    killPid = k;
				    k = MAX_RUNNING;
				    p = MAX_RUNNING;
				}
			    }
			}
		}
	}

	// Sets killPid if process with sufficient resources wasn't found
	if (killPid == -1) killPid = maxPid;

	// This should never happen
	if (killPid == -1) perrorExit("killAProcess - no pid selected");
	if (pidArray[killPid] == EMPTY) perrorExit("killAProcess - bad pid");

	// Kills the process
	killProcess(killPid, pidArray[killPid]);

	// Removes the pid from the array
	pidArray[killPid] = EMPTY;


	
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

// Returns the number of currently running processes
static int numRunning(pid_t * pidArray){
	int i = 0, acc = 0;
	for ( ; i < MAX_RUNNING; i++){
		if (pidArray[i] != EMPTY) acc++;
	}
	return acc;
}

// Detects and resolves deadlock - returns num killed and removes pids
int resolveDeadlock(pid_t * pidArray, ResourceDescriptor * resources,
		    Message * messages){

	int allocated[NUM_RESOURCES * MAX_RUNNING];	// Resource allocation
	int request[NUM_RESOURCES * MAX_RUNNING];	// Current requests
	int available[NUM_RESOURCES];			// Available resources

	int deadlocked[MAX_RUNNING]; // Whether each pid is deadlocked

	int killed = 0;				   // Num terminated processes
	int runningAtStart = numRunning(pidArray); // Num processes running

	// Initializes vectors
	updateMatrices(resources, allocated, request, available, deadlocked);

	// If deadlock exists, repeatedly kills processes until resolved
	bool deadlockDetected = false;
	while(deadlock(available, NUM_RESOURCES, MAX_RUNNING, request,
		       allocated, deadlocked)){

		// Prints resolution message once
		if (!deadlockDetected){
			logResolutionAttempt();
			deadlockDetected = true;
		}

		killAProcess(pidArray, deadlocked, messages, resources);
		killed++;

		// Updates vectors	
		updateMatrices(resources, allocated, request, available, deadlocked);
	}

	if (deadlockDetected)
		logResolutionSuccess(killed, runningAtStart);

	return killed;
}


