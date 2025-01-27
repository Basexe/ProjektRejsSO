#include "common.h"

int main(int argc, char *argv[]) {
    srand(time(NULL));
    signal(SIGINT, handleInterrupt);

    if (argc < 2) {
        fprintf(stderr, "Użycie: %s <liczba_rejsów>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int maxRejs = atoi(argv[1]);
    if (maxRejs <= 0) {
        fprintf(stderr, "[ERROR] Liczba rejsów musi być dodatnia.\n");
        exit(EXIT_FAILURE);
    }

    shmId = shmget(IPC_PRIVATE, sizeof(SharedData), IPC_CREAT | 0600);
    if (shmId < 0) {
        perror("[ERROR] shmget");
        exit(EXIT_FAILURE);
    }

    sharedData = (SharedData *)shmat(shmId, NULL, 0);
    if (sharedData == (void *)-1) {
        perror("[ERROR] shmat");
        cleanupResources();
        exit(EXIT_FAILURE);
    }

    memset(sharedData, 0, sizeof(SharedData));
    sharedData->maxRejs = maxRejs;
    sharedData->passengersUnloaded = 1;

    semId = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    if (semId < 0) {
        perror("[ERROR] semget");
        cleanupResources();
        exit(EXIT_FAILURE);
    }

    if (semctl(semId, MUTEX, SETVAL, 1) < 0) {
        perror("[ERROR] semctl SETVAL");
        cleanupResources();
        exit(EXIT_FAILURE);
    }

    key_t key = ftok(".", 'A');
    msgQueueId = msgget(key, IPC_CREAT | 0666);
    if (msgQueueId < 0) {
        perror("[ERROR] msgget");
        cleanupResources();
        exit(EXIT_FAILURE);
    }

    pid_t kapitanStatkuPid = fork();
    if (kapitanStatkuPid < 0) {
        perror("[ERROR] fork kapitan statku");
        cleanupResources();
        exit(EXIT_FAILURE);
    } else if (kapitanStatkuPid == 0) {
        procesKapitanaStatku();
        exit(EXIT_SUCCESS);
    }

    pid_t kapitanPortuPid = fork();
    if (kapitanPortuPid < 0) {
        perror("[ERROR] fork kapitan portu");
        cleanupResources();
        exit(EXIT_FAILURE);
    } else if (kapitanPortuPid == 0) {
        procesDozorcyPortu(kapitanStatkuPid);
        exit(EXIT_SUCCESS);
    }

    pid_t passengerPids[500];
    int passengerCount = 0;

    while (!sharedData->simulationEnded) {
        if (passengerCount < 500 && sharedData->isLoadingPassengers) {
            pid_t passengerPid = fork();
            if (passengerPid < 0) {
                perror("[ERROR] fork");
                cleanupResources();
                exit(EXIT_FAILURE);
            } else if (passengerPid == 0) {
                procesPasazera(passengerCount + 1);
                exit(EXIT_SUCCESS);
            }
            passengerPids[passengerCount++] = passengerPid;
            usleep(200000);  // 0.2 sekundy
        } else {
            usleep(500000);  // 0.5 sekundy
        }
    }

    for (int i = 0; i < passengerCount; i++) {
        waitpid(passengerPids[i], NULL, 0);
    }
    waitpid(kapitanStatkuPid, NULL, 0);
    waitpid(kapitanPortuPid, NULL, 0);

    cleanupResources();
    printf("[MAIN] Symulacja zakończona.\n");
    return EXIT_SUCCESS;
}