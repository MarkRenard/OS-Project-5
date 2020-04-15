// message.h was created by Mark Renard on 4/11/1989.
//
// This file defines a message used to request resources from oss.

#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdbool.h>
#include "constants.h"

typedef enum msgType {
	VOID, TERMINATION, REQUEST, PENDING_REQUEST, RELEASE
} MsgType;

typedef struct message{
	int simPid;			// The simPid of the sender
	
	int type;			// The type of the message
	int rNum;			// The id of the resource, if applicable
	int quantity;			// The quantity of the resource requested

	int target[NUM_RESOURCES]; 	// Target number of each resource
	int numClassesHeld;	 	// Number of resource classes held

	struct message * previous;	// Previous message in queue
} Message;

void initMessage(Message *, int simPid);
void initMessageArray(Message *);

#endif

