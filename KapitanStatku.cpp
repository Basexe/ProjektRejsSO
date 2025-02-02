#include "common.h"

int handlerSemId = -1;
SharedData* handlerSharedData = NULL;

void handleShipSignals(int sig) {
    lockMutex(handlerSemId);
    if(sig == SIGUSR1) {
        printf("\033[31m[KAPITAN STATKU] Otrzymano sygnał 1 od Kapitana Portu. Statek wypływa wcześniej!\033[0m\n");
        handlerSharedData->shipDeparted = 1;
        handlerSharedData->passengersUnloaded = 0;
        handlerSharedData->isLoadingPassengers = 0;
    }else if(sig == SIGUSR2){
        printf("\033[31m[KAPITAN STATKU] Otrzymano sygnał 2 od Kapitana Portu. Kończę pracę na dzisiaj, jeżeli jestem w trakcie rejsu to go kończę!\033[0m\n");
        handlerSharedData->interrupted=1;
    }
    unlockMutex(handlerSemId);
}

int main() {
    srand(time(NULL) ^ (getpid() << 16));

    signal(SIGUSR1, handleShipSignals);
    signal(SIGUSR2, handleShipSignals);

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
    handlerSharedData = sharedData;

    int semId = semget(Rejs, 1, 0);
    if (semId == -1) {
        perror("[ERROR] semget");
        exit(EXIT_FAILURE);
    }
    handlerSemId = semId;

    int msgQueueId = msgget(Rejs, 0);
    if (msgQueueId < 0) {
        perror("[ERROR] msgget");
        exit(EXIT_FAILURE);
    }

    Message introduce;
    introduce.mtype=3;
    introduce.id=getpid();
    if (msgsnd(msgQueueId, &introduce, sizeof(introduce) - sizeof(long), 0) < 0) {
        perror("[ERROR] msgsnd");
        exit(EXIT_FAILURE);
    }

    while (sharedData->currentRejs < sharedData->maxRejs && !sharedData->interrupted) {
        lockMutex(semId);
        printf("\033[33m[KAPITAN STATKU] Przygotowanie do rejsu nr %d...\033[0m\n", sharedData->currentRejs + 1);
        sharedData->isLoadingPassengers = 1;
        printf("\033[33m[KAPITAN STATKU] Rozpoczynam załadunek pasażerów na rejs %d.\033[0m\n",sharedData->currentRejs + 1);
        unlockMutex(semId);

        int preparationTime = 10;
        for (int i = 0; i < preparationTime && !sharedData->simulationEnded && !sharedData->shipDeparted; i++) {
            sleep(1);

            Message request;
            while (msgrcv(msgQueueId, &request, sizeof(request) - sizeof(long), 1, IPC_NOWAIT) >= 0) {
                printf("\033[33m[KAPITAN STATKU] Otrzymałem prośbę od pasażera %d.\033[0m\n", request.id);

                Message response;
                response.mtype = request.id+10;
                response.id = request.id;
                if (msgsnd(msgQueueId, &response, sizeof(response) - sizeof(long), 0) < 0) {
                    perror("[ERROR] msgsnd");
                    exit(EXIT_FAILURE);
                }
                printf("\033[33m[KAPITAN STATKU] Wysłałem odpowiedź do pasażera %d.\033[0m\n", request.id);
            }
        }
        lockMutex(semId);
        sharedData->isLoadingPassengers = 0;
        unlockMutex(semId);
        int tmp=1;
        while (1)
        {
            lockMutex(semId);
            if (!sharedData->simulationEnded&&sharedData->passengersOnBridge == 0 && sharedData->interrupted) {
                sharedData->shipDeparted = 1;
                sharedData->passengersUnloaded = 0;
                unlockMutex(semId);
                tmp = 0;
                break;
            }
            if (!sharedData->simulationEnded&&sharedData->passengersOnBridge == 0 && !sharedData->interrupted) {
                sharedData->shipDeparted = 1;
                sharedData->passengersUnloaded = 0;
                printf("\033[33m[KAPITAN STATKU] Wypływamy w rejs nr %d z %d pasażerami! (na moście: %d)\033[0m\n",sharedData->currentRejs + 1, sharedData->passengersOnShip, sharedData->passengersOnBridge);
                unlockMutex(semId);
                break;
            }
            unlockMutex(semId);
        }
        if (tmp)
        {
            int rejsTime = 5;
            for (int i = 0; i < rejsTime && !sharedData->simulationEnded; i++) {
                sleep(1);
            }

            lockMutex(semId);
            sharedData->currentRejs++;
            sharedData->shipDeparted = 0;
            sharedData->rejsEnded = 1;
            printf("\033[33m[KAPITAN STATKU] Rejs nr %d zakończony. Rozpoczynam rozładunek.\033[0m\n",sharedData->currentRejs);
        }else{
            lockMutex(semId);
            sharedData->currentRejs++;
            sharedData->shipDeparted = 0;
            sharedData->rejsEnded = 1;
            printf("\033[33m[KAPITAN STATKU] Rejs nr %d nie może się odbyć. Rozpoczynam rozładunek.\033[0m\n",sharedData->currentRejs);
        }

        int waitTime = 0;
        while ((sharedData->passengersOnShip > 0 || sharedData->disembarkingInProgress) &&!sharedData->simulationEnded) {
            printf("\033[33m[KAPITAN STATKU] Czekam na rozładunek pasażerów (pozostało: %d)...\033[0m\n",sharedData->passengersOnShip);
            unlockMutex(semId);

            sleep(1);
            waitTime++;

            if (waitTime > 10) {
                printf("\033[33m[KAPITAN STATKU] Przekroczono czas oczekiwania na rozładunek. Wymuszam zakończenie.\033[0m\n");
                lockMutex(semId);
                sharedData->passengersOnShip = 0;
                sharedData->disembarkingInProgress = 0;
                sharedData->passengersUnloaded = 1;
                unlockMutex(semId);
                break;
            }

            lockMutex(semId);
        }

        sharedData->passengersUnloaded = 1;
        sharedData->rejsEnded = 0;
        unlockMutex(semId);
    }

    printf("\033[33m[KAPITAN STATKU] Wszystkie rejsy zakończone lub symulacja przerwana.\033[0m\n");

    lockMutex(semId);
    sharedData->simulationEnded = 1;
    unlockMutex(semId);

    shmdt(sharedData);
    return 0;
}