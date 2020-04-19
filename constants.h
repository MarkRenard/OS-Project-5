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
#define NUM_RESOURCES 20 		// Total number of resource classes
#define MAX_RUNNING 18 	 		// Max number of running child processes
#define MAX_LAUNCHED 200		// Max total children launched

#define	MIN_INST 1			// Minimum instances of each resource
#define MAX_INST 10			// Maximum instances of each resource

#define SHAREABLE_PROBABILITY 0.2	// Chance resource will be shareable

#define MAX_LOG_LINES 100000		// Max number of lines in the log file
#define ALLOC_PER_TABLE 20		// Number of granted requests per table
 
#define BILLION 1000000000U		// The number of nanoseconds in a second
#define MILLION 1000000U		// Number of nanoseconds per millisecond
#define BUFF_SZ 100			// The size of character buffers 
#define MSG_SZ 30			// Size of Message char arrays

#define EMPTY (-1)			// pidArray value at unassigned index


// Used by oss.c
#define DETECTION_INTERVAL_SEC 1	// Value of seconds per deadlock detection
#define DETECTION_INTERVAL_NS 0		// Nanoseconds per deadlock detection

#define MIN_FORK_TIME_SEC 0U		// Value of seconds in MIN_FORK_TIME
#define MIN_FORK_TIME_NS (1 * MILLION)	// Value of nanoseconds in MIN_FORK_TIME
#define MAX_FORK_TIME_SEC 0U		// Value of seconds in MAX_FORK_TIME
#define MAX_FORK_TIME_NS (500 * MILLION)// Value of nanoseconds in MAX_FORK_TIME

#define LOOP_INCREMENT_SEC 0		// System clock increment per loop sec
#define LOOP_INCREMENT_NS (50 * MILLION)// System clock increment per loop ns

#define SLEEP_NS 500000			// Real sleep between simulation loops

#define USER_PROG_PATH "./userProgram"	// The path to the user program

#define LOG_FILE_NAME "oss_log"		// The name of the output file


// Used by userProgram.c
#define TERMINATION_PROBABILITY 0.1	// Chance of terminating
#define REQUEST_PROBABILITY 0.8		// Chance of request instead of release

#define MIN_CHECK_SEC 0			// Min time between decisions sec
#define MIN_CHECK_NS 0			// Min time between decisions nanosec
#define MAX_CHECK_SEC 0			// Max tmie between decisions sec
#define MAX_CHECK_NS (250 * MILLION) 	// Max time between decisions nanosec

#define MIN_RUN_TIME_SEC 1		// Min run time for user procs sec
#define MIN_RUN_TIME_NS 0		// Min run time for user procs ns

#define CLOCK_UPDATE_SEC 0		// System clock increment for user sec
#define CLOCK_UPDATE_NS (50 * MILLION)	// System clock increment for user ns


// Used by both oss.c and userProgram.c
#define REQUEST_MQ_KEY 59597192	// Message queue key for dispatch
#define REPLY_MQ_KEY 38257848		// Message queue key for interrupts
#define MQ_PERMS (S_IRUSR | S_IWUSR)	// Message queue permissions

#define BASE_SEED 8853984		// Used in calls to srand

#define KILL_MSG "k"			// Message oss uses to kill processes

#endif
