// message.c was created by Mark Renard on 4/11/2020.
//
// This file contains a function which initializes a message used to request
// resources from oss.

#include "message.h"
#include "constants.h"
#include <stdio.h>
// Initializes a single message to default values
void initMessage(Message * msg, int simPid){
	msg->simPid = simPid;

	msg->type = VOID;
	
	int i = 0;
	for( ; i < NUM_RESOURCES; i++)
		msg->target[i] = 0;

	msg->quantity = 0;
	msg->numClassesHeld = 0;
}

// Initializes the shared array of messages to default values
void initMessageArray(Message * msgArr){
	int i;
	for (i = 0; i < MAX_RUNNING; i++){
		fprintf(stderr, "Attempting to initialize msg for P%d\n", i);
		initMessage(&msgArr[i], i);
	}
}
