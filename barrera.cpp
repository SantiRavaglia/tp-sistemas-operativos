#include "barrera.h"
#include <mutex>
#include <semaphore.h>


barrera::barrera(int i) {
    sem_init(semaforo, 0, 0);
    cant = 0;
    trigger = i;
}

void barrera::wait() {
    m.lock();

    cant++;
    printf("   ++++++++++++   Voy por cant=%d", cant);
    if (cant >= trigger) {
        for (int i = 0; i < trigger; i++) {
            sem_post(semaforo);
        }
        cant = 0;
    }

    m.unlock();

    sem_wait(semaforo);

}