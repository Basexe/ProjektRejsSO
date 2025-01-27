#include "common.h"

void procesPasazera(int id) {
    srand(time(NULL) ^ (getpid() << 16));

    while (1) {
        lockMutex();
        if (sharedData->simulationEnded) {
            printf("[PASAŻER %d] Symulacja zakończona. Kończę działanie.\n", id);
            unlockMutex();
            exit(EXIT_SUCCESS);
        }

        if (sharedData->passengersOnBridge < MAX_ON_BRIDGE && 
            sharedData->passengersUnloaded && 
            sharedData->isLoadingPassengers) {
            sharedData->passengersOnBridge++;
            printf("[PASAŻER %d] Wszedłem na mostek. Liczba pasażerów na moście: %d\n", 
                   id, sharedData->passengersOnBridge);
            unlockMutex();
            break;
        } else {
            if (!sharedData->isLoadingPassengers) {
                printf("[PASAŻER %d] Czekam na rozpoczęcie załadunku.\n", id);
            } else {
                printf("[PASAŻER %d] Nie mogę wejść na mostek, bo jest za dużo ludzi.\n", id);
            }
        }

        unlockMutex();
        sleep(1);
    }

    sleep(rand() % 2 + 1);

    Message request;
    request.mtype = 1;
    request.id = id;
    if (msgsnd(msgQueueId, &request, sizeof(request) - sizeof(long), 0) < 0) {
        perror("[ERROR] msgsnd");
        exit(EXIT_FAILURE);
    }
    printf("[PASAŻER %d] Wysłałem prośbę do kapitana o wejście na statek.\n", id);

    Message response;
    if (msgrcv(msgQueueId, &response, sizeof(response) - sizeof(long), 2, 0) < 0) {
        perror("[ERROR] msgrcv");
        exit(EXIT_FAILURE);
    }
    printf("[PASAŻER %d] Otrzymałem odpowiedź od kapitana: mogę wejść na statek.\n", id);

    lockMutex();
    if (!sharedData->shipDeparted && 
        sharedData->passengersUnloaded && 
        sharedData->passengersOnShip < MAX_ON_SHIP && 
        sharedData->isLoadingPassengers) {
        sharedData->passengersOnShip++;
        sharedData->passengersOnBridge--;
        printf("[PASAŻER %d] Wszedłem na statek. Liczba pasażerów na statku: %d\n", 
               id, sharedData->passengersOnShip);
        unlockMutex();
    } else {
        printf("[PASAŻER %d] Nie mogę wejść na statek, bo rejs już się rozpoczął lub nie został rozładowany.\n", id);
        unlockMutex();
        exit(EXIT_SUCCESS);
    }

    while (1) {
        lockMutex();
        if (sharedData->rejsEnded) {
            printf("[PASAŻER %d] Opuszczam statek.\n", id);
            sharedData->passengersOnShip--;
            sharedData->disembarkingInProgress = 1;
            
            if (sharedData->passengersOnShip == 0) {
                printf("[SYSTEM] Wszyscy pasażerowie opuścili statek.\n");
                sharedData->disembarkingInProgress = 0;
                sharedData->passengersUnloaded = 1;
            }
            unlockMutex();
            break;
        }

        if (sharedData->simulationEnded) {
            printf("[PASAŻER %d] Symulacja zakończona. Kończę działanie.\n", id);
            unlockMutex();
            exit(EXIT_SUCCESS);
        }

        unlockMutex();
        sleep(1);
    }

    exit(EXIT_SUCCESS);
}