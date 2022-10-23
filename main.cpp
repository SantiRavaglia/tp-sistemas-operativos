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

const estrategia strat = SHORTEST;
const int quantum = 10;

int main(){
    Config config = *(new Config());

	cout << "0" << endl;
    gameMaster belcebu = gameMaster(config);
    cout << "1" << endl;

	// Creo equipos (lanza procesos)
	int quantum = 10;
	
    Equipo rojo(&belcebu, ROJO, strat, config.cantidad_jugadores, quantum, config.pos_rojo);
	cout << "2" << endl;
    Equipo azul(&belcebu, AZUL, strat, config.cantidad_jugadores, quantum, config.pos_azul);
    
    // while(true) {
    //     sleep(10);
    //     cout << "sleep +++++++++++++++++++++++++++++++++++" << endl;
    // }
    sleep(1);
    rojo.comenzar();
    sleep(3);
    azul.comenzar();
    sleep(1);
	rojo.terminar();
	azul.terminar();
    //belcebu.play();
    cout << "Bandera capturada por el equipo "<< belcebu.ganador << ". Felicidades!" << endl;

}

