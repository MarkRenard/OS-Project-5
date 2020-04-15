// constants.h was created by Mark Renard on 3/26/2020
//
// This file contains definitions of constants used in assignment 5 of the
// spring 2020 semester of 4760, grouped according to the source files in which
// they are used.

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <sys/msg.h>
#include <sys/stat.h>


// Miscelaneous
#define NUM_RESOURCES 4			// Total number of resource classes

#define	MIN_INST 1			// Minimum instances of each resource
#define MAX_INST 10			// Maximum instances of each resource

#define EMPTY -1			// pidArray value at unassigned index
 
#define BILLION 1000000000U		// The number of nanoseconds in a second
#define MILLION 1000000U		// Number of nanoseconds per millisecond
#define BUFF_SZ 100			// The size of character buffers 
#define MSG_SZ 30			// Size of Message char arrays

#define MAX_LOG_LINES 100000		// Max number of lines in the log file

// Used by oss.c
#define MAX_RUNNING 5	 		// Max number of running child processes
#define MAX_LAUNCHED 5 			// Max total children launched

#define SHAREABLE_PROBABILITY 0.2	// Chance resource will be shareable

#define MIN_FORK_TIME_SEC 0U		// Value of seconds in MIN_FORK_TIME
#define MAX_FORK_TIME_NS 500000000U	// Value of nanoseconds in MAX_FORK_TIME

#define USER_PROG_PATH "./userProgram"	// The path to the user program

#define LOOP_INCREMENT_SEC 0		// System clock increment per loop sec
#define LOOP_INCREMENT_NS 100000	// System clock increment per loop ns

#define LOG_FILE_NAME "oss_log"		// The name of the output file

#define MAX_SECONDS 3		 	// Max total execution time of oss


// Used by userProgram.c
#define TERMINATION_PROBABILITY 0.1	// Chance of terminating
#define REQUEST_PROBABILITY 0.5		// Chance of request instead of release

#define MIN_RES 1			// Min quantity to request or release
#define MAX_RES 10			// Max quantity to request or release

#define MIN_WAIT_SEC 0			// Min seconds waiting if blocked
#define MIN_WAIT_MS 0			// Min milliseconds waiting if blocked
#define MAX_WAIT_SEC 3			// Max seconds waiting if blocked
#define MAX_WAIT_MS 1000		// Max milliseconds waiting if blocked


// Used by both oss.c and userProgram.c
#define DISPATCH_MQ_KEY 59597192	// Message queue key for dispatch
#define REPLY_MQ_KEY 38257848		// Message queue key for interrupts
#define MQ_PERMS (S_IRUSR | S_IWUSR)	// Message queue permissions

#define BASE_SEED 8853984		// Used in calls to srand

#define TERMINATION_CH 't'		// Indicates process termination
#define USES_ALL_QUANTUM_CH 'u'		// Indicates process used entire quantum
#define WAITING_FOR_IO_CH 'w'		// Indicates process is waiting for I/O
#define PREEMPT_CH 'p'			// Indicates process was preempted
#define DELIM ' '			// Message text delimiter

// Used by bitVector.c
#define NUM_BITS (sizeof(unsigned int) * 8)  // Bits per unsigned int
#define MAX_VALUE (MAX_BLOCKS - 1) 	   // Max int tracked in bit vector
#define BIT_VECTOR_SIZE (MAX_VALUE / NUM_BITS + 1) // Size of bit vector


// Used by multiQueue.c 
#define PROMOTION_WAIT_TIME_THRESHOLD_NS 0 // Aging criterion for promotion sec
#define PROMOTION_WAIT_TIME_THRESHOLD_SEC MAX_BLOCKS // Aging criterion seconds
#define UTIL_THRESHOLD (1 / (long double) multiQ->count) // Cpu use criterion

#define WAKE_UP_INCREMENT_SEC 0		// Blocked process wake-up seconds
#define WAKE_UP_INCREMENT_NS 100	// Blocked process wake-up nanosecodns



#endif
