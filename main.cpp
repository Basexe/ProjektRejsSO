#include "common.h"

int handlerSemId = -1;
SharedData* handlerSharedData = NULL;

void handlesigint(int) {
    lockMutex(handlerSemId);
    handlerSharedData->simulationEnded = 1;
    unlockMutex(handlerSemId);
}

int main(int argc, char *argv[]) {
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags   = SA_NOCLDWAIT;
    sigaction(SIGCHLD, &sa, NULL);

    signal(SIGINT, handlesigint);

    if (argc < 2) {
        fprintf(stderr, "Użycie: %s <liczba_rejsów>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int maxRejs = atoi(argv[1]);
    if (maxRejs <= 0) {
        perror("[ERROR] Liczba rejsów musi być dodatnia.");
        exit(EXIT_FAILURE);
    }

    key_t Rejs = ftok("./",1);
    if (Rejs == -1) {
        perror("[ERROR] ftok");
        exit(EXIT_FAILURE);
    }

    int shmId = shmget(Rejs, sizeof(SharedData), IPC_CREAT | 0600);
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

    // Initialize shared data
    sharedData->passengersOnBridge = 0;
    sharedData->passengersOnShip = 0;
    sharedData->currentRejs = 0;
    sharedData->maxRejs = maxRejs;
    sharedData->shipDeparted = 0;
    sharedData->passengersUnloaded = 1;
    sharedData->rejsEnded = 0;
    sharedData->simulationEnded = 0;
    sharedData->interrupted = 0;
    sharedData->departEarly = 0;
    sharedData->disembarkingInProgress = 0;
    sharedData->isLoadingPassengers = 0;

    int semId = semget(Rejs, 1, IPC_CREAT | 0600);
    if (semId == -1) {
        perror("[ERROR] semget");
        exit(EXIT_FAILURE);
    }
    handlerSemId = semId;

    if (semctl(semId, MUTEX, SETVAL, 1) < 0) {
        perror("[ERROR] semctl SETVAL");
        exit(EXIT_FAILURE);
    }

    int msgQueueId = msgget(Rejs, IPC_CREAT | 0666);
    if (msgQueueId < 0) {
        perror("[ERROR] msgget");
        exit(EXIT_FAILURE);
    }

    pid_t shipCaptainPid = fork();
    if (shipCaptainPid == 0){
        execlp("./KapitanStatku","KapitanStatku",NULL);
        perror("[ERROR] fork");
        exit(EXIT_FAILURE);
    }

    pid_t portCaptainPid = fork();
    if (portCaptainPid == 0){
        execlp("./KapitanPortu","KapitanPortu",NULL);
        perror("[ERROR] fork");
        exit(EXIT_FAILURE);
    }

    pid_t passengerPids[500];
    int passengerCount = 0;

    while (!sharedData->simulationEnded) {
        pid_t passengerPid = fork();
        if (passengerPid == 0)
        {
            char buffor[16];
            sprintf(buffor, "%d", passengerCount+1);
            execlp("./Pasazer","Pasazer",buffor,NULL);
            perror("[ERROR] fork");
            exit(EXIT_FAILURE);
        }
        passengerPids[passengerCount++] = passengerPid;

        sleep(1);
    }

    for (int i = 0; i < passengerCount; i++) {
        waitpid(passengerPids[i], NULL, 0);
    }

    waitpid(shipCaptainPid, NULL, 0);
    waitpid(portCaptainPid, NULL, 0);

    printf("[MAIN] Symulacja zakończona.\n");

    shmctl(shmId, IPC_RMID, NULL);
    semctl(semId, 0, IPC_RMID, 0);
    msgctl(msgQueueId, IPC_RMID, NULL);
    shmdt(sharedData);
    return 0;
}