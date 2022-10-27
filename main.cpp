#include <iostream>
#include <thread>
#include <sys/unistd.h>
#include <vector>
#include <mutex>
#include "gameMaster.h"
#include "equipo.h"
#include "definiciones.h"
#include "config.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

const estrategia strat = USTEDES;
const int quantum = 10;

int main(){
    Config config = *(new Config());

	
    gameMaster belcebu = gameMaster(config);

	// Creo equipos (lanza procesos)
	int quantum = 10;
	
    Equipo rojo(&belcebu, ROJO, strat, config.cantidad_jugadores, quantum, config.pos_rojo);
    Equipo azul(&belcebu, AZUL, strat, config.cantidad_jugadores, quantum, config.pos_azul);
    
    // while(true) {
    //     sleep(10);
    //     cout << "sleep +++++++++++++++++++++++++++++++++++" << endl;
    // }
    sleep(1);
    rojo.comenzar();
    azul.comenzar();
	rojo.terminar();
	azul.terminar();
    // belcebu.play();
    cout << "Bandera capturada por el equipo "<< belcebu.ganador << ". Felicidades!" << endl;

}

