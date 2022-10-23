#ifndef EQUIPO_H
#define EQUIPO_H

#include <semaphore.h>
#include <vector>
#include <thread>
#include "definiciones.h"
#include "gameMaster.h"
// #include <barrier>

using namespace std;

class Equipo {
	private:

		// Atributos Privados 
		gameMaster *belcebu; 
		color contrario, equipo, bandera_contraria;
		estrategia strat;
		int cant_jugadores, quantum, quantum_restante;
		vector<thread> jugadores;
		int cant_jugadores_que_ya_jugaron = 0;
		vector<coordenadas> posiciones;
		coordenadas pos_bandera_contraria;
		//
		// ...
		//
		//barrier barrera_jugadores(1);
		//barrera *barrera_jugadores = new barrera();

		// Métodos privados 
		direccion apuntar_a(coordenadas pos2, coordenadas pos1);
		void jugador(int nro_jugador);
		coordenadas buscar_bandera_contraria(int casillaInicio, int cantCasillas);
		coordenadas buscar_bandera_contraria_single_thread();
		int jugador_mas_cercano();
		//
		// ...
		//
	public:
		Equipo(gameMaster *belcebu, color equipo, 
				estrategia strat, int cant_jugadores, int quantum, vector<coordenadas> posiciones);
		void comenzar();
		void terminar();
		mutex m_turno;
		// crear jugadores

};
#endif // EQUIPO_H
