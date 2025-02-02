#include "common.h"

void lockMutex(int semId) {
    struct sembuf op = {MUTEX, -1, 0};
    while (1) {
        if (semop(semId, &op, 1) == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                perror("[ERROR] semop lockMutex");
                exit(EXIT_FAILURE);
            }
        }
        break;
    }
}

void unlockMutex(int semId) {
    struct sembuf op = {MUTEX, 1, 0};
    while (1) {
        if (semop(semId, &op, 1) == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                perror("[ERROR] semop unlockMutex");
                exit(EXIT_FAILURE);
            }
        }
        break;
    }
}
