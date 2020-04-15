// userProgram.c was created by Mark Renard on 4/12/2020
//
// This program sends messages to an operating system simulator, simulating a
// process that requests and relinquishes resources at random times.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#include "constants.h"
#include "getSharedMemoryPointers.h"
#include "perrorExit.h"
#include "protectedClock.h"
#include "qMsg.h"
#include "randomGen.h"
#include "sharedMemory.h"

// Prototypes
static void signalTermination(int simPid);
static bool requestResources(ResourceDescriptor *, Message *, int);
static bool releaseResources(ResourceDescriptor *, Message *, int);
static int getRandomRNum();

// Constants
static const struct timespec SLEEP = {0, 1000};
static const Clock MIN_INC = {0, 0};
static const Clock MAX_INC = {0, 250 * MILLION};

// Static global
static char * shm;			// Shared memory region pointer
static int targetHeld[NUM_RESOURCES];	// Number of each resource to be held

static int requestMqId;			// Message queue id of request queue
static int replyMqId;			// Message queue id of reply queue

int main(int argc, char * argv[]){
	exeName = argv[0];		// Sets exeName for perrorExit
	int simPid = atoi(argv[1]);	// Gets process's logical pid
	srand(BASE_SEED + simPid); 	// Seeds pseudorandom number generator

        ProtectedClock * systemClock;	// Shared memory system clock
        ResourceDescriptor * resources;	// Shared memory resource table
        Message * messages;		// Shared memory message vector

	Clock decisionTime;		// Time to request, relese, or terminate
	Clock startTime;		// Time the process started
	Clock now;			// Temp storage for time

	fprintf(stderr, "\n\tPROCESS P%d RUNNING!\n", simPid);

	getSharedMemoryPointers(&shm, &systemClock, &resources, &messages, 0);

/*
	fprintf(stderr, "\tP%d SHM ADDRESS: %p\n", simPid, shm);
	fprintf(stderr, "\tP%d CLOCK: %p\n", simPid, systemClock);
	fprintf(stderr, "\tP%d RESOURCES: %p\n", simPid, resources);
	fprintf(stderr, "\tP%d MESSAGES ADDRESS: %p\n\n", simPid, messages);
*/

	// Initializes clocks
	startTime = getPTime(systemClock);
	decisionTime = startTime;
	fprintf(stderr, "\tP%d DECISION TIME - %03d : %09d\n\n",
		simPid, decisionTime.seconds, decisionTime.nanoseconds);

	// Gets message queues
        requestMqId = getMessageQueue(DISPATCH_MQ_KEY, MQ_PERMS | IPC_CREAT);
        replyMqId = getMessageQueue(REPLY_MQ_KEY, MQ_PERMS | IPC_CREAT);
	char reply[BUFF_SZ];

	// Repeatedly requests or releases resources or terminates
	bool terminating = false;
	bool msgSent = false;	
	while (!terminating) {

		// Decides when current time is at or after decision time
		now = getPTime(systemClock);
		if (clockCompare(now, decisionTime) >= 0){

			fprintf(stderr, "\n\tP%d - MAKING DECISION AT " \
				"%03d : %09d\n\n", simPid, now.seconds, 
				now.nanoseconds);

			// Updates decision time
			incrementClock(&decisionTime, 
					randomTime(MIN_INC, MAX_INC));

			// Decides whether to terminate
			if (randBinary(TERMINATION_PROBABILITY)){
				signalTermination(simPid);
				terminating = true;
				msgSent = true;

			// Decides whether to request or release resources
			} else if (randBinary(REQUEST_PROBABILITY)){
				msgSent = requestResources(resources, messages, simPid);
			} else {
				msgSent = releaseResources(resources, messages, simPid);
			}	
		}

		// Waits for response to request
		if (msgSent){
			fprintf(stderr, "\n\tP%d WAITING FOR MSG\n\n", simPid);
			waitForMessage(replyMqId, reply, simPid);
		}
	}

	// Prepares to exit
	detach(shm);
	fprintf(stderr, "\n\tPROCESS P%d COMPLETING!\n\n", simPid);

	return 0;
}

static void signalTermination(int simPid){
	fprintf(stderr, "\n\tPROCESS %d SIGNALING TERMINATION\n\n", simPid);

	char msgBuff[BUFF_SZ];
	sprintf(msgBuff, "0");
	sendMessage(requestMqId, msgBuff, simPid + 1);
}

// Sends a message over a message queue requesting random resources
static bool requestResources(ResourceDescriptor * resources, 
			     Message * messages, int simPid){

	fprintf(stderr, "\n\tPROCESS %d - requestResources\n\n", simPid);

	char msgBuff[BUFF_SZ];	// Message buffer
	int rNum;		// Resource index
	int maxRequest;		// Max quantity of requested resources
	int quantity;		// Actual quantity requested
	int encoded;		// Encoded message

	// Randomly selects a resource to request
	rNum = randInt(0, NUM_RESOURCES - 1);

	// Computes maximum request
	maxRequest = resources[rNum].numInstances - targetHeld[rNum];

	// Returns if none can be requested
	if (maxRequest == 0) return false;

	// Randomly selects quantity to request
	quantity = randInt(1, maxRequest);

	// Records new target
	targetHeld[rNum] += quantity;

	// Endcodes resource index and quantity in a message
	encoded = (MAX_INST + 1) * rNum + quantity;

	// Sends the message
	sprintf(msgBuff, "%d", encoded);
	sendMessage(requestMqId, msgBuff, simPid + 1);

	fprintf(stderr, "\n\tPROCESS %d REQUESTING %d OF R%d", simPid,
		quantity, rNum);
	fprintf(stderr, "\n\tPROCESS %d REQUESTING %d OF R%d", simPid,
		encoded % (MAX_INST + 1), encoded / (MAX_INST + 1));

	return true;

}

// Sends a message over a message queue releasing random resources
static bool releaseResources(ResourceDescriptor * resources,
			     Message * messages, int simPid){

	fprintf(stderr, "\n\tPROCESS %d, releaseResources", simPid);
	char msgBuff[BUFF_SZ];	// Message buffer
	int rNum;		// Resource index
	int quantity;		// Actual quantity requested
	int encoded;		// Encoded message
	
	// Selects a held resource at random or returns if no resources held
	if ((rNum = getRandomRNum()) == -1){
		fprintf(stderr, "\n\tPROCESS %d CAN'T RELEASE ANYTHING\n\n",
			simPid);
		 return false;
	}

	// Randomly determines quantity to release
	quantity = randInt(1, targetHeld[rNum]);

	// Records new target
	targetHeld[rNum] -= quantity;

	// Encodes resource index and quantity in a message
	encoded = -((MAX_INST + 1) * rNum + quantity);

	// Sends the message
	sprintf(msgBuff, "%d", encoded);
	sendMessage(requestMqId, msgBuff, simPid + 1);

	fprintf(stderr, "\n\tPROCESS %d RELEASING %d OF R%d", simPid,
		quantity, rNum);
	fprintf(stderr, "\n\tPROCESS %d RELEASING %d OF R%d", simPid,
		encoded % (MAX_INST + 1), encoded / (MAX_INST + 1));

	return true;

}

// Gets a randomly chosen index of a held resource or -1 if no resources held
static int getRandomRNum(){
	int resourceCount;		// Number of resource classes held
	int resInd[NUM_RESOURCES]; // Indices of held resources

	// Records the indecies of held resources in resInd
	int i = 0, j = 0;
	for ( ; i < NUM_RESOURCES; i++)
		if (targetHeld[i] > 0) resInd[j++] = i;
	resourceCount = j;

	// Returns -1 if no resoures are held
	if (resourceCount == 0) return -1;

	// Returns the index of the randomly chosen resource
	return resInd[randInt(0, resourceCount - 1)];
}
	
/*
	Abandonded Implementation

// Creates a request in shared memory for a random number of a random resource
static void requestResources(ResourceDescriptor * resources, Message * messages,
			     int simPid){

	// Randomly selects the resource descriptor of a resource to request
	int rNum = randInt(0, NUM_RESOURCES - 1);

	// Calculates maximum request quantity
	int allocated = resources[rNum].allocations[simPid];
	int maxRequest = resources[rNum].numInstances - allocated;

	// Makes the request if more can be requested
	if (maxRequest > 0){

		// Increments num classes if no resources of this class held
		if (messages[simPid].target[rNum] == 0)
			messages[simPid].numClassesHeld++;

		// Adds a requested number of instances to the target number
		int numRequested = randInt(1, maxRequest);
		messages[simPid].quantity = numRequested;
		messages[simPid].target[rNum] += numRequested;
		messages[simPid].rNum = rNum;

		fprintf(stderr, "\n\tPROCESS %d REQUESTING %d OF R%d\n",
			simPid, numRequested, rNum);

		messages[simPid].type = REQUEST;
		fprintf(stderr, "\tMSG[%d].TYPE: %d ADDRESS: %p\n\n", 
			simPid, messages[simPid].type, &(messages[simPid].type));
	}

}

// Sends message to release a random number of a random resource
static void releaseResources(ResourceDescriptor * resources, Message * messages,
			     int simPid){

	// Returns if no resources are held
	if (messages[simPid].numClassesHeld == 0) return;

	// Selects the index of a held resource randomly
	int rNum = selectResourceIndex(&messages[simPid], simPid);

	// Constructs message to release a random number of the chosen resource
	int numHeld = messages[simPid].target[rNum];
	int numReleased = randInt(1, numHeld);
	messages[simPid].quantity = numReleased;
	messages[simPid].target[rNum] -= numReleased;
	messages[simPid].rNum = rNum;

	// Decrements num classes if no resources of this class will be held
	if (messages[simPid].target[rNum] == 0)
		messages[simPid].numClassesHeld--;

	fprintf(stderr, "\n\tPROCESS %d RELEASING %d OF R%d\n\n",
		simPid, numReleased, rNum);

	messages[simPid].type = RELEASE;

}

// Randomly selects the index of a currently held resource in the request
static int selectResourceIndex(const Message * msg, int simPid){

	// Selects which held resource to release
	int classNum = randInt(1, msg->numClassesHeld);

	// Finds the rNum of that resource by checking held resource classes 
	int rNum = 0, checked = 0;
	for ( ; rNum < NUM_RESOURCES; rNum++){

		// Increments checked if resource at current rNum is held
		if (msg->target[rNum] != 0){
			checked++;		

			// Breaks if classNum reached
			if (checked == classNum) break;
		}
	}

	return rNum;
} */
