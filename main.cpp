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
const tipo_busqueda busqueda = MULTI_THREAD; // elegir entre MULTI_THREAD o SINGLE_THREAD

int main(){
    Config config = *(new Config());

    gameMaster belcebu = gameMaster(config);

	// Creo equipos (lanza procesos)
	int quantum = 22;
	
    Equipo rojo(&belcebu, ROJO, strat, busqueda, config.cantidad_jugadores, quantum, config.pos_rojo);
    Equipo azul(&belcebu, AZUL, strat, busqueda, config.cantidad_jugadores, quantum, config.pos_azul);
    

    rojo.comenzar();
    azul.comenzar();
	rojo.terminar();
	azul.terminar();
    cout << "Bandera capturada por el equipo "<< (belcebu.ganador == 0 ? "Azul" : "Rojo") << ". Felicidades!" << endl;

}

