// message.h was created by Mark Renard on 4/11/1989.
//
// This file defines a message used to request resources from oss.

#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdbool.h>
#include "constants.h"

typedef struct message{
	bool pending;
	int requested[NUM_RESOURCES];
} Message;

void initMessage(Message *);

#endif

