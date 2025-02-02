#include "common.h"

int main()
{
    srand(time(NULL) ^ getpid()^20);

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

    Message introduction;
    introduction.id=-1;
    while (1){
        if (msgrcv(msgQueueId, &introduction, sizeof(introduction) - sizeof(long), 3, 0) <= 0){
            if (errno == EINVAL) continue;
            perror("[ERROR] msgrcv");
        }
        printf("\033[32m[KAPITAN PORTU] Kapitan Statku się przedstawił.\033[0m\n");
        break;
    }

    pid_t shipCapitanPid = introduction.id;

    int randValue = rand() % 100 + 1;

    while (1)
    {
        lockMutex(semId);
        if (sharedData->simulationEnded) {
            unlockMutex(semId);
            break;
        }
        unlockMutex(semId);

        lockMutex(semId);
        if (sharedData->isLoadingPassengers){
            randValue = rand() % 100 + 1;
            if (randValue <= SIGNAL1_CHANCE)
            {
                kill(shipCapitanPid, SIGUSR1);
                printf("\033[31m[KAPITAN PORTU] Wysyłam sygnał 1.\033[0m\n");
            }
        }
        unlockMutex(semId);

        sleep(2);

        lockMutex(semId);
        if (!sharedData->interrupted){
            randValue = rand() % 100 + 1;
            if (randValue >= 100-SIGNAL2_CHANCE){
                kill(shipCapitanPid, SIGUSR2);
                printf("\033[31m[KAPITAN PORTU] Wysyłam sygnał 2.\033[0m\n");
            }
        }
        unlockMutex(semId);
        sleep(2);
    }

    printf("\033[32m[KAPITAN PORTU] Wszystkie rejsy zakończone lub symulacja przerwana.\033[0m\n");
    shmdt(sharedData);
    return 0;
}
