// queue.c was created by Mark Renard on 2/5/2020 and modified on 4/17/2020.
// This file defines functions that operate on a queue of messages.


#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "perrorExit.h"
#include <stdio.h>

// Sets the values of pointers to null and count to 0
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
	if (msg->currentQueue != NULL)
		perrorExit("addToFront on msg with non-null currentQueue");

	msg->next = NULL;
	msg->previous = q->front;

	if (msg->previous != NULL)
		msg->previous->next = msg;

	q->front = msg;

	// Adds to back as well if previously empty
	if (q->back == NULL) q->back = msg;
	q->count++;
	
}

// Adds a message to the back of the queue
void enqueue(Queue * q, Message * msg){

	if (msg->currentQueue != NULL)
		perrorExit("Tried to enqueue msg with non-null currentQueue");

	msg->currentQueue = q;

	// Adds message to queue	
	if (q->back != NULL){

		// Connects to back of queue if queue is not empty
		q->back->previous = msg;
		msg->next = q->back;
	} else {
		// Adds to front if queue is empty
		q->front = msg;
		msg->next = NULL;
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
	if (q->front != NULL)
		q->front->next = NULL;	// Front of queue shouldn't have a next
	else
		q->back = NULL;	// Back is null if queue is empty

	// Removes links from dequeued element
	returnVal->previous = NULL; 
	returnVal->next = NULL;
	returnVal->currentQueue = NULL;

	q->count--;
	return returnVal;

}

// Removes a particular element from any place in its queue
void removeFromCurrentQueue(Message * msg){
	Queue * q = msg->currentQueue;

	if (q == NULL) 
		perrorExit("Tried to remove msg when not in queue");

	if (q->front == NULL || q->back == NULL || q->count < 1)
		perrorExit("Tried to remove msg from empty queue");

	// Finds new front if msg is the front
	if (q->front == msg)
		q->front = msg->previous;

	// Finds new back if msg is the back
	if (q->back == msg)
		q->back = msg->next;

	// Connects previous node to next node
	if (msg->next != NULL)
		msg->next->previous = msg->previous;

	// Connects next node to previous node
	if (msg->previous != NULL)
		msg->previous->next = msg->next;

	// Sets msg queue pointers to null
	msg->next = NULL;
	msg->previous = NULL;
	msg->currentQueue = NULL;
	
	q->count--;
}
