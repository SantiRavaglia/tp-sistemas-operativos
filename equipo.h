#ifndef EQUIPO_H
#define EQUIPO_H

#include <semaphore.h>
#include <vector>
#include <thread>
#include "definiciones.h"
#include "gameMaster.h"
#include <barrier>
#include <time.h>

using namespace std;

class Equipo {
	private:

		// Atributos Privados 
		gameMaster *belcebu; 
		color contrario, equipo, bandera_contraria;
		estrategia strat;
		tipo_busqueda busqueda;
		int cant_jugadores, quantum, quantum_restante;
		vector<thread> jugadores, threads_bandera;
		int cant_jugadores_que_ya_jugaron = 0;
		vector<coordenadas> posiciones;
		coordenadas pos_bandera_contraria;

		pair<int, int> tiempo_busqueda;
		struct timespec busqueda_inicio;
		struct timespec busqueda_fin;
		struct timespec strat_inicio;
		struct timespec strat_fin;
		pair<int, int> tiempo_strat;


		// Métodos privados 
		direccion apuntar_a(coordenadas pos2, coordenadas pos1);
		void jugador(int nro_jugador);
		coordenadas buscar_bandera_contraria(int nro_jugador);
		coordenadas buscar_bandera_contraria_single_thread();
		int jugador_mas_cercano();
		//
		// ...
		//

	public:

		vector<bool> ya_jugo;

		Equipo(gameMaster *belcebu, color equipo, 
				estrategia strat, tipo_busqueda busqueda, int cant_jugadores, int quantum, vector<coordenadas> posiciones);
		void comenzar();
		void terminar();
		mutex m_turno;
		// crear jugadores

};
#endif // EQUIPO_H
