// oss.c was created by Mark Renard on 4/11/2020.
//
// This program simulates deadlock detection and resolution.

#include "clock.h"
#include "deadlockDetection.h"
#include "getSharedMemoryPointers.h"
#include "logging.h"
#include "message.h"
#include "perrorExit.h"
#include "pidArray.h"
#include "protectedClock.h"
#include "qMsg.h"
#include "queue.h"
#include "resourceDescriptor.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Prototypes
static void simulateResourceManagement();
static pid_t launchUserProcess(int simPid);
void processTerm(int, bool killed);
static void processRequest(int);
static void processRelease(int);
static void processQueuedRequests(int rNum);
static void processAllQueuedRequests();
static void grantRequest(Message * msg);
static int parseMessage();
static void assignSignalHandlers();
static void cleanUpAndExit(int param);
static void cleanUp();

// Constants
static const Clock DETECTION_INTERVAL = {1, 0};
static const Clock MIN_FORK_TIME = {MIN_FORK_TIME_SEC, MIN_FORK_TIME_NS};
static const Clock MAX_FORK_TIME = {MAX_FORK_TIME_SEC, MAX_FORK_TIME_NS};
static const Clock MAIN_LOOP_INCREMENT = {0, 50 * MILLION};
static const struct timespec SLEEP = {0, 500000};

// Static global variables
static char * shm;				// Pointer to the shared memory region
static ProtectedClock * systemClock;		// Shared memory system clock
static ResourceDescriptor * resources;		// Shared memory resource table
static Message * messages;			// Shared memory message vector

static int requestMqId;	// Id of message queue for resource requests & release
int replyMqId;		// Id of message queue for replies from oss

int main(int argc, char * argv[]){

	exeName = argv[0];	// Assigns exeName for perrorExit
	assignSignalHandlers(); // Sets response to ctrl + C & alarm
//	alarm(MAX_SECONDS);	// Limits total execution time
	openLogFile();		// Opens file written to in logging.c

	srand(BASE_SEED - 1);   // Seeds pseudorandom number generator

	// Creates shared memory region and gets pointers
	getSharedMemoryPointers(&shm, &systemClock, &resources, &messages, 
				IPC_CREAT);
        // Creates message queues
        requestMqId = getMessageQueue(DISPATCH_MQ_KEY, MQ_PERMS | IPC_CREAT);
        replyMqId = getMessageQueue(REPLY_MQ_KEY, MQ_PERMS | IPC_CREAT);

	// Initializes system clock and shared arrays
	initPClock(systemClock);
	initResources(resources);
	initMessageArray(messages);
	
	// Generates processes, grants requests, and resolves deadlock in a loop
	simulateResourceManagement();

	cleanUp();

	return 0;
}

// Generates processes, grants requests, and resolves deadlock in a loop
void simulateResourceManagement(){

	Clock timeToFork = zeroClock();		 // Time to launch user process 
	Clock timeToDetect = DETECTION_INTERVAL; // Time to resolve deadlock

	pid_t pidArray[MAX_RUNNING];		// Array of user process pids
	initPidArray(pidArray);			// Sets pids to -1
	pid_t simPid;				// Temporary pid storage

	int running = 0;			// Currently running child count
	int launched = 0;			// Total children launched

	int i = 0, m;				// Loop control variables

	// Launches processes and resolves deadlock until limits reached
	do {

		fprintf(stderr, "\nIteration %d:\n", i++);
		printPids(pidArray);

		// Waits for semaphore protecting system clock
		pthread_mutex_lock(&systemClock->sem);		

		// Launches user processes at random times
		if (clockCompare(systemClock->time, timeToFork) >= 0){
			 
			// Launches process & records real pid if within limits
			if (running < MAX_RUNNING && launched < MAX_LAUNCHED){
				simPid = getLogicalPid(pidArray);
				pidArray[simPid] = launchUserProcess(simPid);

				running++;
				launched++;
			}

			// Selects new random time to launch a new user process
			incrementClock(&timeToFork, randomTime(MIN_FORK_TIME,
							       MAX_FORK_TIME));
		}

		// Responds to new messages from the queue
		while ((m = parseMessage()) != -1){
			if (messages[m].type == REQUEST){
				processRequest(m);
			} else if (messages[m].type == RELEASE) {
				processRelease(m);
			} else if (messages[m].type == TERMINATION){
				processTerm(m, false);
			
				// Removes from running processes
				pidArray[m] = EMPTY;
				running--;
			}
		}
/*
		// Detects and resolves deadlock at regular intervals
		if (clockCompare(systemClock->time, timeToDetect) >= 0){
			logDeadlockDetection(systemClock->time);

			running -= resolveDeadlock(pidArray, resources, 
						   messages);

			incrementClock(&timeToDetect, DETECTION_INTERVAL);
		}

*/
		incrementClock(&systemClock->time, MAIN_LOOP_INCREMENT);
		printTimeln(stderr, systemClock->time);

		nanosleep(&SLEEP, NULL);

		// Unlocks the system clock
		pthread_mutex_unlock(&systemClock->sem);

	} while ((running > 0 || launched < MAX_LAUNCHED) && i < 100);
}

// Forks & execs a user process with the assigned logical pid, returns child pid
static pid_t launchUserProcess(int simPid){

	pid_t realPid;

	// Forks, exiting on error
	if ((realPid = fork()) == -1) perrorExit("Failed to fork");

	// Child process calls execl on the user program binary
	if (realPid == 0){
		char sPid[BUFF_SZ];
		sprintf(sPid, "%d", simPid);
		
		execl(USER_PROG_PATH, USER_PROG_PATH, sPid, NULL);
		perrorExit("Failed to execl");
	}


	fprintf(stderr, "launchUserProcess(%d) called - real pid: %d\n", 
		simPid, realPid);

	return realPid;

}

// Parses & returns the pid of a newly received message, or -1 if there are none
static int parseMessage(){
	char msgText[MSG_SZ];	// Raw text of each message
	int msgInt;		// Integer form of each message
	int rNum;		// The index of the resource
	int quantity;		// Quantity requested or released

	long int qMsgType;	// Raw type of msg
	int simPid;		// simPid of sender

	if (getMessage(requestMqId, msgText, &qMsgType)){
		simPid = (int)(qMsgType - 1);	// Subtract 1 to get simPid
		msgInt = atoi(msgText);		// Converts to encoded int

		// Parses release messages
		if (msgInt < 0){
			quantity = -msgInt % (MAX_INST + 1);
			rNum = -msgInt / (MAX_INST + 1);
			messages[simPid].type = RELEASE;

			fprintf(stderr, "Receiving that P%d released %d of R%d\n",
				simPid, quantity, rNum);

		// Parses request messages
		} else if (msgInt > 0){
			quantity = msgInt % (MAX_INST + 1);
			rNum = msgInt / (MAX_INST + 1);
			messages[simPid].type = REQUEST;

			logRequestDetection(simPid, rNum, quantity, 
					    systemClock->time);

			fprintf(stderr, "Receiving that P%d requested %d of R%d\n",
				simPid, quantity, rNum);

		// Parses termination messages
		} else {
			messages[simPid].type = TERMINATION;

			fprintf(stderr, "Receiving that P%d terminated\n",
				simPid);

			logCompletion(simPid);

			return simPid;
		}

		messages[simPid].quantity = quantity;
		messages[simPid].rNum = rNum;

		return simPid;
	}else{
		return -1;
	}
}

// Gets message
// static void parseMessages(Message * messages);

// Responds to a termination event by releasing associated resources
void processTerm(int simPid, bool killed){

	fprintf(stderr, "Responding to termination of process %d\n", simPid);

	int released[NUM_RESOURCES];

	// Frees all resources previously allocated to the process
	int r;
	for (r = 0; r < NUM_RESOURCES; r++){
		released[r] = resources[r].allocations[simPid];
		resources[r].numAvailable += resources[r].allocations[simPid];
		resources[r].allocations[simPid] = 0;
	}

	logRelease(released, NUM_RESOURCES);

	// Processes old requests for released resources
	for (r = 0; r < NUM_RESOURCES; r++){
		if (released[r] > 0) processQueuedRequests(r);
	}

	// Resets message
	initMessage(&messages[simPid], simPid);

	// Replies with acknowlegement if the process was not killed
	if (!killed){
		sendMessage(replyMqId, "termination confirmed", simPid + 1);
	}
}

// Responds to a request for resources by granting it or enqueueing the request
static void processRequest(int simPid){
	Message * msg = &messages[simPid]; // The message to respond to

	// Grants request if it is less than available
	if (msg->quantity <= resources[msg->rNum].numAvailable){
		grantRequest(msg);

	// Enqueues message otherwise
	} else {
		// Logs request denial
		fprintf(stderr, "Can't meet P%d request for %d of R%d, in q\n",
			simPid, msg->quantity, msg->rNum);
		logEnqueue(simPid, msg->quantity, msg->rNum, 
			resources[msg->rNum].numAvailable);

		enqueue(&resources[msg->rNum].waiting, msg);
		msg->type = PENDING_REQUEST;
	}
}

// Examines request queues and grants old requests for available resources
static void processQueuedRequests(int rNum){
	Message * msg;				// Stores each queued message	
	Queue * q = &resources[rNum].waiting;	// The queue to process
	int qCount = q->count;			// Initial number in queue

	int i = 0;
	for ( ; i < qCount; i++){
		msg = q->front;

		// Grants request if possible
		if (msg->quantity <= resources[rNum].numAvailable){

			fprintf(stderr, "Granting old request from P%d for " \
				"%d of R%d\n", msg->simPid, msg->quantity,
				 msg->rNum);

			grantRequest(msg);
			dequeue(q);

		// Re-enqueues if not
		} else {
			fprintf(stderr, "Can't grant queued request from P%d for " \
				"%d of R%d - only %d available\n", msg->simPid, 
				msg->quantity, msg->rNum, 
				resources[rNum].numAvailable);
			dequeue(q);
			enqueue(q, msg);
		}
	}
}

// Grants a request for resources
static void grantRequest(Message * msg){
	fprintf(stderr, "Granting P%d request for %d of R%d\n",
		msg->simPid, msg->quantity, msg->rNum);

	// Increeases allocation and decreases availablity
	resources[msg->rNum].allocations[msg->simPid] += msg->quantity;
	resources[msg->rNum].numAvailable -= msg->quantity;

	// Prints granted request to log file
	logAllocation(msg->simPid, msg->rNum, msg->quantity, 
		      systemClock->time);

	// Logs resource table every 20 granted requests by default
	logTable(resources);

	// Resets msg
	msg->quantity = 0;
	msg->type = VOID;

	// Replies with acknowlegement
	sendMessage(replyMqId, "request confirmed", msg->simPid + 1);
}

// Calls processQueuedRequest on all resource numbers
static void processAllQueuedRequests(){
	int i = 0;
	for ( ; i < NUM_RESOURCES; i++){
		processQueuedRequests(i);
	}
}

// Releases resources from a process
static void processRelease(int simPid){
	Message * msg = &messages[simPid];

	logResourceRelease(simPid, msg->rNum, msg->quantity, 
			   systemClock->time);

	resources[msg->rNum].allocations[simPid] -= msg->quantity;
	resources[msg->rNum].numAvailable += msg->quantity;

	msg->quantity = 0;
	msg->type = VOID;

	processAllQueuedRequests();

	// Replies with acknowlegement
	sendMessage(replyMqId, "release confirmed", simPid + 1);
}

// Determines the processes response to ctrl + c or alarm
static void assignSignalHandlers(){
	struct sigaction sigact;

	// Initializes sigaction values
	sigact.sa_handler = cleanUpAndExit;
	sigact.sa_flags = 0;

	// Assigns signals to sigact
	if ((sigemptyset(&sigact.sa_mask) == -1)
	    ||(sigaction(SIGALRM, &sigact, NULL) == -1)
	    ||(sigaction(SIGINT, &sigact, NULL)  == -1)){

		// Prints error message and exits on failure
		char buff[BUFF_SZ];
		sprintf(buff, "%s: Error: Failed to install signal handlers",
			exeName);
		perror(buff);
		exit(1);
	}
}

// Signal handler - closes files, removes shm, terminates children, and exits
static void cleanUpAndExit(int param){

	// Closes files, removes shm, terminates children
	cleanUp();

	// Prints error message
	char buff[BUFF_SZ];
	sprintf(buff,
		 "%s: Error: Terminating after receiving a signal",
		 exeName
	);
	perror(buff);

	// Exits
	exit(1);
}

// Ignores interrupts, kills child processes, closes files, removes shared mem
static void cleanUp(){
	// Handles multiple interrupts by ignoring until exit
	signal(SIGALRM, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	// Kills all other processes in the same process group
	kill(0, SIGQUIT);

	// Removes message queues
	removeMessageQueue(requestMqId);
	removeMessageQueue(replyMqId);

	// Detatches from and removes shared memory
	detach(shm);
	removeSegment();
}

