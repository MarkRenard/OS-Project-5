OSS	= oss
OSS_OBJ	= $(COMMON_O) oss.o pidArray.o logging.o deadlockDetection.o \
	  matrixRepresentation.o
OSS_H	= $(COMMON_H) oss.h pidArray.h logging.h deadlockDetection.h \
	  matrixRepresentation.o

USER_PROG	= userProgram
USER_PROG_OBJ	= $(COMMON_O) userProgram.o 
USER_PROG_H	= $(COMMON_H) 

COMMON_O   = $(UTIL_O) getSharedMemoryPointers.o protectedClock.o \
	     resourceDescriptor.o message.o qMsg.o queue.o
COMMON_H   = $(UTIL_H) getSharedMemoryPointers.h protectedClock.h constants.h \
	     resourceDescriptor.h message.h qMsg.h queue.h

UTIL_O	   = clock.o perrorExit.o randomGen.o sharedMemory.o
UTIL_H	   = clock.h perrorExit.h randomGen.h sharedMemory.h shmkey.h

OUTPUT     = $(OSS) $(USER_PROG) 
OUTPUT_OBJ = $(OSS_OBJ) $(USER_PROG_OBJ)
CC         = gcc
FLAGS      = -g -lm -lpthread $(DEBUG) $(VB) -Wall 

DEBUG	   = #-DDEBUG -DDEBUG_Q -DDEBUG_SHM -DDEBUG_USER

VB	   = #-DVERBOSE

.SUFFIXES: .c .o

all: $(OUTPUT)

$(OSS): $(OSS_OBJ) $(OSS_H)
	$(CC) $(FLAGS) -o $@ $(OSS_OBJ) 

$(USER_PROG): $(USER_PROG_OBJ) $(USER_PROG_H)
	$(CC) $(FLAGS) -o $@ $(USER_PROG_OBJ) 

.c.o:
	$(CC) $(FLAGS) -c $<

.PHONY: clean rmfiles cleanall
clean:
	/bin/rm -f $(OUTPUT) $(OUTPUT_OBJ)
rmfiles:
	/bin/rm -f oss_log
cleanall:
	/bin/rm -f oss_log $(OUTPUT) $(OUTPUT_OBJ)


