// protectedClock.h was created by Mark Renard on 4/11/2020.
//
// This file defines a type used to extend the base clock type to allow
// mutually exclusive access by multiple processes for writing.

#ifndef PROTECTEDCLOCK_H
#define PROTECTEDCLOCK_H

#include <stdio.h>
#include <pthread.h>
#include "clock.h"

typedef struct protectedClock {
	Clock time;
	pthread_mutex_t sem;
} ProtectedClock;

void initPClock(ProtectedClock * pClockPtr);
void incrementPClock(ProtectedClock * pClockPtr, Clock increment);
Clock getPTime(ProtectedClock * pClockPtr);

/*
// Uses two unsigned ints as a clock
typedef struct clock {
	unsigned int seconds;
	unsigned int nanoseconds;
} Clock;




Clock zeroClock();
Clock newClock(unsigned int seconds, unsigned int nanoseconds);
Clock randomTime(Clock min, Clock max);
void copyTime(Clock * dest, const Clock src);
void incrementClock(Clock * clock, const Clock increment);
int clockCompare(const Clock clk1, const Clock clk2);
Clock clockSum(Clock t1, Clock t2);
Clock clockDiff(Clock t1, Clock t2);
long double clockRatio(Clock t1, Clock t2);
Clock clockQuotient(Clock t1, Clock t2);
Clock clockDiv(Clock time, int divisor);
void printTime(FILE * fp, const Clock clock);
void printTimeln(FILE * fp, const Clock clock);
*/

#endif
