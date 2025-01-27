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
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define MAX_ON_BRIDGE 5
#define MAX_ON_SHIP 20
#define MUTEX 0

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

// Zmienne globalne
extern int shmId;
extern int semId;
extern int msgQueueId;
extern SharedData *sharedData;
extern int semaphoresActive;

// Deklaracje funkcji
void lockMutex();
void unlockMutex();
void cleanupResources();
void handleInterrupt(int sig);

// Deklaracje procesów
void procesPasazera(int id);
void procesKapitanaStatku();
void procesDozorcyPortu(pid_t kapitanStatkuPid);

#endif