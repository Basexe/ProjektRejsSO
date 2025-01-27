#include "common.h"

// Definicje zmiennych globalnych
int shmId = -1;
int semId = -1;
int msgQueueId = -1;
SharedData *sharedData = NULL;
int semaphoresActive = 1;

void lockMutex() {
    if (!semaphoresActive) return;
    struct sembuf op = {MUTEX, -1, 0};
    if (semop(semId, &op, 1) < 0) {
        if (errno != EINVAL) {
            perror("[ERROR] semop lockMutex");
            exit(EXIT_FAILURE);
        }
    }
}

void unlockMutex() {
    if (!semaphoresActive) return;
    struct sembuf op = {MUTEX, 1, 0};
    if (semop(semId, &op, 1) < 0) {
        if (errno != EINVAL) {
            perror("[ERROR] semop unlockMutex");
            exit(EXIT_FAILURE);
        }
    }
}

void cleanupResources() {
    if (shmId >= 0) {
        if (shmctl(shmId, IPC_RMID, NULL) < 0) {
            perror("[ERROR] shmctl IPC_RMID");
        }
    }
    if (semId >= 0) {
        semaphoresActive = 0;
        if (semctl(semId, 0, IPC_RMID) < 0) {
            perror("[ERROR] semctl IPC_RMID");
        }
    }
    if (msgQueueId >= 0) {
        if (msgctl(msgQueueId, IPC_RMID, NULL) < 0) {
            perror("[ERROR] msgctl IPC_RMID");
        }
    }
}

void handleInterrupt(int sig) {
    lockMutex();
    if (sig == SIGINT) {
        printf("*************************************************************\n");
        printf("[KAPITAN] Otrzymano SIGINT (Ctrl+C). Statek dopływa do brzegu...\n");
        printf("*************************************************************\n");
        sharedData->simulationEnded = 1;
        sharedData->shipDeparted = 0;
        sharedData->rejsEnded = 1;
    } else if (sig == SIGUSR1) {
        printf("[KAPITAN] Otrzymano SIGUSR1. Statek wypływa wcześniej!\n");
        sharedData->shipDeparted = 1;
        sharedData->passengersUnloaded = 0;
    } else if (sig == SIGUSR2) {
        printf("[KAPITAN] Otrzymano SIGUSR2. Specjalny event!\n");
        if (!sharedData->shipDeparted) {
            printf("[KAPITAN] Pasażerowie opuszczają statek. Program kończy działanie.\n");
            sharedData->simulationEnded = 1;
            sharedData->rejsEnded = 1;
        } else {
            printf("[KAPITAN] Rejs kończy się normalnie.\n");
            sharedData->rejsEnded = 1;
        }
    }
    unlockMutex();
}