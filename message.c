// message.c was created by Mark Renard on 4/11/2020.
//
// This file contains a function which initializes a message used to request
// resources from oss.

#include "message.h"
#include "constants.h"

// Initializes pending to false and requested to 0 for each resource
void initMessage(Message * msg){
	msg->pending = false;
	
	int i = 0;
	for( ; i < NUM_RESOURCES; i++)
		msg->requested[i] = 0;

}
