// oss.h was created by Mark Renard on 4/15/2020.
//
// This file exports the replyMqId variable to deadlockDetection.c so that
// killAProcess can send a message to a user process.

#ifndef OSS_H
#define OSS_H

#include <sys/types.h>

int replyMqId;
void processTerm(int simPid, bool killed);
void killProcess(int simPid, pid_t realPid);

#endif
