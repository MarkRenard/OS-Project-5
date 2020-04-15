// queue.c was created by Mark Renard on 2/5/2020 and modified on 4/14/2020.
// This file defines functions that operate on a queue of messages.


#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "perrorExit.h"
#include <stdio.h>

void initializeQueue(Queue * qPtr){
	qPtr->front = NULL;
	qPtr->back = NULL;
	qPtr->count = 0;
}

// Prints the simPid of messages in the queue from front to back
void printQueue(FILE * fp, const Queue * q){
	Message * msg = q->front;

	while (msg != NULL){
		fprintf(fp, " %02d", msg->simPid);
		msg = msg->previous;
	}
}

// Adds a message to the front of the queue
void addToFront(Queue * q, Message * msg){
	msg->previous = q->front;
	q->front = msg;

	// Adds to back as well if previously empty
	if (q->back == NULL) q->back = msg;
	q->count++;
	
}

// Adds a message to the back of the queue
void enqueue(Queue * q, Message * msg){
#ifdef DEBUG_Q
	fprintf(stderr, "\tenqueue(%02d): ", msg->simPid);
	printQueue(stderr, q);
	fprintf(stderr, " <- %02d\n", msg->simPid);
#endif
	// Adds message to queue	
	if (q->back != NULL){
		// Adds to previous msg of back if queue is not empty
		q->back->previous = msg;
	} else {
		// Adds to front if queue is empty
		q->front = msg;
	}
	q->back = msg;

	// Back of queue shouldn't have a previous element
	q->back->previous = NULL;

	// Increments node count in queue
	q->count++;

}

// Removes and returns Message reference from the front of the queue
Message * dequeue(Queue * q){

	// Exits with an error message if queue is empty
	if (q->count <= 0 || q->front == NULL || q->back == NULL)
		perrorExit("Called dequeue on empty queue");

	// Assigns current front of queue to returnVal
	Message * returnVal = q->front;  	
	
	// Removes the front node from the queue
	q->front = q->front->previous; // Assigns new front of queue
	returnVal->previous = NULL; // Removes previous from dequeued block
		
	// Sets queue back to null if empty
	if(q->front == NULL) q->back = NULL;
	
#ifdef DEBUG_Q
	fprintf(stderr, "\tdequeue(): %02d <-", returnVal->simPid);
	printQueue(stderr, q);
	fprintf(stderr, "\n");
#endif

	q->count--;
	return returnVal;


}


