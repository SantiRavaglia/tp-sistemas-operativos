#include <iostream>
#include <thread>
#include <sys/unistd.h>
#include <vector>
#include <mutex>
#include "gameMaster.h"
#include "equipo.h"
#include "definiciones.h"
#include "config.h"

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
    
    
    rojo.comenzar();
    azul.comenzar();
	rojo.terminar();
	azul.terminar();
    //belcebu.play();
    cout << "1" << endl;
    cout << "Bandera capturada por el equipo "<< belcebu.ganador << ". Felicidades!" << endl;

}

