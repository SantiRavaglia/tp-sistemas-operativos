#include <semaphore.h>
#include <mutex>

using namespace std;

class barrera {
    public:

        barrera(int i);
        void wait();
    
    private:
        sem_t *semaforo;
        mutex m;
        int cant;
        int trigger;
};