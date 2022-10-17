#include <semaphore.h>
#include <mutex>

using namespace std;

class barrera {
    public:

        barrera();
        barrera(int i);
        barrera(barrera& b_aux);
        barrera& operator=(const barrera& b_aux) {
            return *this;
        };
        void wait();
    
    private:
        sem_t *semaforo;
        mutex m;
        int cant;
        int trigger;
};