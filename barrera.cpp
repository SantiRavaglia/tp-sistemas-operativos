#include "barrera.h"
#include <mutex>
#include <semaphore.h>

barrera::barrera() {
    
}

barrera::barrera(int i) {
    sem_init(semaforo, 0, 0);
    this->cant = 0;
    this->trigger = i;
}

barrera::barrera(barrera& b_aux) {
    sem_init(this->semaforo, 0, 0);
    this->cant = b_aux.cant;
    this->trigger = b_aux.trigger;
}

void barrera::wait() {
    m.lock();

    cant++;
    if (cant >= trigger) {
        for (int i = 0; i < trigger; i++) {
            sem_post(semaforo);
        }
        cant = 0;
    }

    m.unlock();

    sem_wait(semaforo);
}