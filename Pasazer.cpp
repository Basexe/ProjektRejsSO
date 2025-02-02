#include "common.h"

int main(int argc, char *argv[])
{
    srand(time(NULL) ^ (getpid() << 16));

    if (argc < 2) {
        perror("[ERROR] Błąd argc");
        exit(EXIT_FAILURE);
    }

    int id = atoi(argv[1]);
    if (id <= 0) {
        perror("[ERROR] Złe id pasażera");
        exit(EXIT_FAILURE);
    }

    int tmp1 = 0;
    int tmp2 = 0;
    int tmp3 = 0;

    key_t Rejs = ftok("./",1);
    if (Rejs == -1) {
        perror("[ERROR] ftok");
        exit(EXIT_FAILURE);
    }

    int shmId = shmget(Rejs, sizeof(SharedData), 0);
    if (shmId == -1) {
        perror("[ERROR] shmget");
        exit(EXIT_FAILURE);
    }

    SharedData* sharedData = (SharedData*)shmat(shmId, NULL, 0);
    if (sharedData == (void *)-1) {
        perror("[ERROR] shmat");
        exit(EXIT_FAILURE);
    }

    int semId = semget(Rejs, 1, 0);
    if (semId == -1) {
        perror("[ERROR] semget");
        exit(EXIT_FAILURE);
    }

    int msgQueueId = msgget(Rejs, 0);
    if (msgQueueId < 0) {
        perror("[ERROR] msgget");
        exit(EXIT_FAILURE);
    }
    while (1) {
        tmp3=0;
        while (1) {
            lockMutex(semId);
            if (sharedData->simulationEnded) {
                printf("[PASAŻER %d] Symulacja zakończona. Kończę działanie.\n", id);
                unlockMutex(semId);
                shmdt(sharedData);
                return 0;
            }

            if (sharedData->passengersOnBridge <= MAX_ON_BRIDGE &&
                sharedData->passengersUnloaded &&
                sharedData->isLoadingPassengers) {
                sharedData->passengersOnBridge++;
                printf("[PASAŻER %d] Wszedłem na mostek. \033[90mLiczba pasażerów na moście: %d\033[0m\n",
                       id, sharedData->passengersOnBridge);
                unlockMutex(semId);
                break;
                } else {
                    if (!sharedData->isLoadingPassengers) {
                        if (tmp1 == 0) printf("[PASAŻER %d] Czekam na rozpoczęcie załadunku.\n", id);
                        tmp1 = 1;
                    } else {
                        if (tmp2 == 0) printf("[PASAŻER %d] Nie mogę wejść na mostek, bo jest za dużo ludzi.\n", id);
                        tmp2 = 1;
                    }
                }

            unlockMutex(semId);
            usleep(10000);
        }

        usleep(700000);
        Message request;
        request.mtype = 1;
        request.id = id;
        if (msgsnd(msgQueueId, &request, sizeof(request) - sizeof(long), 0) < 0) {
            perror("[ERROR] msgsnd");
            exit(EXIT_FAILURE);
        }
        printf("[PASAŻER %d] Wysłałem prośbę do kapitana o wejście na statek.\n", id);

        Message response;
        while (1)
        {
            lockMutex(semId);
            if (!sharedData->isLoadingPassengers){
                printf("[PASAŻER %d] Załadunek się skończył, schodzę z mostku i czekam na załadunek.\n", id);
                sharedData->passengersOnBridge--;
                unlockMutex(semId);
                tmp3=1;
                break;
            }

            int res = msgrcv(msgQueueId, &response, sizeof(response) - sizeof(long), id+10, IPC_NOWAIT);

            if (res >= 0) {
                printf("[PASAŻER %d] Otrzymałem odpowiedź od kapitana: mogę wejść na statek.\n", id);
                break;
            }

            if (sharedData->simulationEnded) {
                printf("[PASAŻER %d] Symulacja zakończona. Kończę działanie.\n", id);
                unlockMutex(semId);
                shmdt(sharedData);
                return 0;
            }
            unlockMutex(semId);
        }
        if (tmp3) continue;

        if (!sharedData->shipDeparted &&
            sharedData->passengersUnloaded &&
            sharedData->passengersOnShip <= MAX_ON_SHIP &&
            sharedData->isLoadingPassengers) {
            sharedData->passengersOnShip++;
            sharedData->passengersOnBridge--;
            printf("[PASAŻER %d] Wszedłem na statek. \033[94mLiczba pasażerów na statku: %d\033[0m\n",
                id, sharedData->passengersOnShip);
            unlockMutex(semId);
            } else {
                printf("[PASAŻER %d] Nie mogę wejść na statek, bo rejs już się rozpoczął lub nie został rozładowany.\n", id);
                sharedData->passengersOnBridge--;
                unlockMutex(semId);
                continue;
                }

        if (tmp3) continue;
        break;
    }

    while (1) {
        lockMutex(semId);
        if (sharedData->rejsEnded) {
            printf("[PASAŻER %d] Opuszczam statek.\n", id);
            sharedData->passengersOnShip--;
            sharedData->disembarkingInProgress = 1;

            if (sharedData->passengersOnShip == 0) {
                printf("[SYSTEM] Wszyscy pasażerowie opuścili statek.\n");
                sharedData->disembarkingInProgress = 0;
                sharedData->passengersUnloaded = 1;
            }
            unlockMutex(semId);
            break;
        }

        if (sharedData->simulationEnded) {
            printf("[PASAŻER %d] Symulacja zakończona. Kończę działanie.\n", id);
            unlockMutex(semId);
            shmdt(sharedData);
            return 0;
        }

        unlockMutex(semId);
        sleep(1);
    }
    shmdt(sharedData);
    return 0;
}