#include "common.h"

void procesDozorcyPortu(pid_t kapitanStatkuPid) {
    srand(time(NULL) ^ (getpid() << 16));

    while (!sharedData->simulationEnded) {
        lockMutex();
        if (sharedData->isLoadingPassengers) {
            // Szansa na SIGUSR1 podczas załadunku (25%)
            if (rand() % 100 < 25) {
                printf("[KAPITAN PORTU] Wysyłam sygnał SIGUSR1 (wcześniejsze wypłynięcie).\n");
                kill(kapitanStatkuPid, SIGUSR1);
                usleep(500000); // Krótka przerwa po wysłaniu sygnału
            }

            // Szansa na SIGUSR2 podczas załadunku (25%)
            if (rand() % 100 < 25) {
                printf("[KAPITAN PORTU] Wysyłam sygnał SIGUSR2 (specjalny event podczas załadunku).\n");
                kill(kapitanStatkuPid, SIGUSR2);
                usleep(500000); // Krótka przerwa po wysłaniu sygnału
            }
        }

        if (sharedData->shipDeparted) {
            // Szansa na SIGUSR2 podczas rejsu (25%)
            if (rand() % 100 < 25) {
                printf("[KAPITAN PORTU] Wysyłam sygnał SIGUSR2 (specjalny event podczas rejsu).\n");
                kill(kapitanStatkuPid, SIGUSR2);
                usleep(500000); // Krótka przerwa po wysłaniu sygnału
            }
        }
        unlockMutex();
        usleep(1000000); // 1 sekunda przerwy między sprawdzeniami
    }

    printf("[KAPITAN PORTU] Kończę pracę.\n");
}