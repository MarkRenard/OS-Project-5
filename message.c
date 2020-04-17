// message.c was created by Mark Renard on 4/11/2020.
//
// This file contains functions for initializing and resetting message
// structures used to encode requests to oss.

#include "constants.h"
#include "queue.h"
#include "message.h"

#include <stdio.h>

// Sets the request values to 0
static void zeroHeld(Message * msg){
	msg->type = VOID;
	msg->quantity = 0;

	int i = 0;
	for( ; i < NUM_RESOURCES; i++)
		msg->target[i] = 0;
	msg->numClassesHeld = 0;
}

// Initializes a single message to default values
void initMessage(Message * msg, int simPid){

	// Request paraeters
	msg->simPid = simPid;
	msg->type = VOID;
	msg->quantity = 0;

	int i = 0;
	for( ; i < NUM_RESOURCES; i++)
		msg->target[i] = 0;
	msg->numClassesHeld = 0;

	// Initializes queue values
	msg->currentQueue = NULL;
	msg->next = NULL;
	msg->previous = NULL;
}

// Initializes the shared array of messages to default values
void initMessageArray(Message * msgArr){
	int i;
	for (i = 0; i < MAX_RUNNING; i++){
		fprintf(stderr, "Attempting to initialize msg for P%d\n", i);
		initMessage(&msgArr[i], i);
	}
}

// Returns a message to its state before it was assigned to a process
void resetMessage(Message * msg){
	msg->type = VOID;
	msg->quantity = 0;

	int i = 0;
	for( ; i < NUM_RESOURCES; i++)
		msg->target[i] = 0;
	msg->numClassesHeld = 0;

	if (msg->currentQueue != NULL)
		removeFromCurrentQueue(msg);
}
