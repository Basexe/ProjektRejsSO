#include "common.h"

void procesKapitanaStatku() {
    signal(SIGINT, handleInterrupt);
    signal(SIGUSR1, handleInterrupt);
    signal(SIGUSR2, handleInterrupt);

    while (sharedData->currentRejs < sharedData->maxRejs && !sharedData->interrupted) {
        printf("[KAPITAN] Przygotowanie do rejsu nr %d...\n", sharedData->currentRejs + 1);

        // Czekamy na opuszczenie statku przez wszystkich pasażerów
        lockMutex();
        int waitTime = 0;
        while ((sharedData->passengersOnShip > 0 || sharedData->disembarkingInProgress) && 
               !sharedData->simulationEnded) {
            printf("[KAPITAN] Czekam na rozładunek pasażerów (pozostało: %d)...\n", 
                   sharedData->passengersOnShip);
            unlockMutex();
            
            sleep(1);
            waitTime++;
            
            if (waitTime > 10) {
                printf("[KAPITAN] Przekroczono czas oczekiwania na rozładunek. Wymuszam zakończenie.\n");
                lockMutex();
                sharedData->passengersOnShip = 0;
                sharedData->disembarkingInProgress = 0;
                sharedData->passengersUnloaded = 1;
                unlockMutex();
                break;
            }
            
            lockMutex();
        }

        // Rozpoczynamy nowy proces załadunku
        sharedData->isLoadingPassengers = 1;
        sharedData->passengersUnloaded = 1;
        sharedData->rejsEnded = 0;
        printf("[KAPITAN] Rozpoczynam załadunek pasażerów na rejs %d.\n", 
               sharedData->currentRejs + 1);
        unlockMutex();

        int preparationTime = 10;
        for (int i = 0; i < preparationTime && !sharedData->simulationEnded; i++) {
            sleep(1);

            Message request;
            while (msgrcv(msgQueueId, &request, sizeof(request) - sizeof(long), 1, IPC_NOWAIT) >= 0) {
                printf("[KAPITAN] Otrzymałem prośbę od pasażera %d.\n", request.id);

                Message response;
                response.mtype = 2;
                response.id = request.id;
                if (msgsnd(msgQueueId, &response, sizeof(response) - sizeof(long), 0) < 0) {
                    perror("[ERROR] msgsnd");
                    exit(EXIT_FAILURE);
                }
                printf("[KAPITAN] Wysłałem odpowiedź do pasażera %d.\n", request.id);
            }
        }

        lockMutex();
        if (!sharedData->simulationEnded) {
            sharedData->isLoadingPassengers = 0;
            sharedData->shipDeparted = 1;
            sharedData->passengersUnloaded = 0;
            printf("[KAPITAN] Wypływamy w rejs nr %d z %d pasażerami!\n", 
                   sharedData->currentRejs + 1, sharedData->passengersOnShip);
        }
        unlockMutex();

        // Czas rejsu
        int rejsTime = 5;
        for (int i = 0; i < rejsTime && !sharedData->simulationEnded; i++) {
            sleep(1);
        }

        lockMutex();
        sharedData->currentRejs++;
        sharedData->shipDeparted = 0;
        sharedData->rejsEnded = 1;
        printf("[KAPITAN] Rejs nr %d zakończony. Rozpoczynam rozładunek.\n", 
               sharedData->currentRejs);
        unlockMutex();
    }

    printf("[KAPITAN] Wszystkie rejsy zakończone lub symulacja przerwana.\n");
    
    lockMutex();
    sharedData->simulationEnded = 1;
    unlockMutex();
}