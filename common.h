#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <errno.h>

// Constants
#define MAX_ON_BRIDGE 15
#define MAX_ON_SHIP 25
#define MUTEX 0

#define SIGNAL1_CHANCE 10
#define SIGNAL2_CHANCE 2

// Structures
typedef struct {
    int passengersOnBridge;
    int passengersOnShip;
    int currentRejs;
    int maxRejs;
    int shipDeparted;
    int passengersUnloaded;
    int rejsEnded;
    int simulationEnded;
    int interrupted;
    int departEarly;
    int disembarkingInProgress;
    int isLoadingPassengers;
} SharedData;

typedef struct {
    long mtype;
    int id;
} Message;

// Mutex operations
void lockMutex(int semId);
void unlockMutex(int semId);

#endif // COMMON_H