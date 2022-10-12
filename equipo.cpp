#include "equipo.h"
#include <assert.h>     /* assert */
#include "definiciones.h"

direccion Equipo::apuntar_a(coordenadas pos1, coordenadas pos2) {
	if (pos2.first > pos1.first) return ABAJO;
	if (pos2.first < pos1.first) return ARRIBA;
	if (pos2.second > pos1.second) return DERECHA;
	else if (pos2.second < pos1.second) return IZQUIERDA;
}


void Equipo::jugador(int nro_jugador) {
	//
	// ...
	//

	while(!this->belcebu->termino_juego()) { // Chequear que no haya una race condition en gameMaster
		switch(this->strat) {
			//SECUENCIAL,RR,SHORTEST,USTEDES
			case(SECUENCIAL):
				this->belcebu->m_turno.lock();
				if (cant_jugadores_que_ya_jugaron == cant_jugadores) {
					this->belcebu->termino_ronda(this->equipo); //HAY QUE DESBLOQUEAR EL MUTEX
					// Hay que refreshear el quantum/jugadores que ya jugaron al principio de la ronda
				} else {
					this->belcebu->mover_jugador(apuntar_a(), nro_jugador);
					cant_jugadores_que_ya_jugaron++;
				}
				this->belcebu->m_turno.unlock();
				break;
			
			case(RR):
			// Quizas vaya aca el semaforo y la resta de quantum donde va?
				this->belcebu->mover_jugador("DIRECCION", nro_jugador);
				if(quantum_restante == 0){
					quantum_restante = quantum;
					this->belcebu->termino_ronda(this->equipo);
				}
				break;

			case(SHORTEST):
			// 	Tengo que ver de donde saco la dist a la bandera
			// Si es un movimiento random o hacia la bandera 
			// Si esta es la strat tengo que asegurarme de llamar al de menor dist
				this->belcebu->mover_jugador("DIRECCION", nro_jugador);
				this->belcebu->termino_ronda(this->equipo);
				break;

			case(USTEDES):
				//
				// ...
				//
				break;
			default:
				break;
		}	
		// Termino ronda ? Recordar llamar a belcebu...
		// OJO. Esto lo termina un jugador... 
		//
		// ...
		//
	}
	
}

Equipo::Equipo(gameMaster *belcebu, color equipo, 
		estrategia strat, int cant_jugadores, int quantum, vector<coordenadas> posiciones) {
	this->belcebu = belcebu;
	this->equipo = equipo;
	this->contrario = (equipo == ROJO)? AZUL: ROJO;
	this->bandera_contraria = (equipo==ROJO)? BANDERA_AZUL: BANDERA_ROJA;
	this->strat = strat;
	this->quantum = quantum;
	this->quantum_restante = quantum;
	this->cant_jugadores = cant_jugadores;
	this->posiciones = posiciones;
	//
	// ...
	//
}

void Equipo::comenzar() {
	// Arranco cuando me toque el turno 
	// TODO: Quien empieza ? No se si lo de abajo esta bien
	//fsem_wait(&(belcebu->turno_rojo)); // Inicializo el rojo
	sem_wait(&(belcebu->turno_azul)); // Pongo a esperar el azul y cuando termine el rojo va a iniciar este
	// Creamos los jugadores
	for(int i=0; i < cant_jugadores; i++) {
		jugadores.emplace_back(thread(&Equipo::jugador, this, i)); 
	}
}

void Equipo::terminar() {
	for(auto &t:jugadores){
		t.join();
	}	
}

coordenadas Equipo::buscar_bandera_contraria() {
	//
	// ...
	//

	assert(this->equipo == ROJO && this->equipo == AZUL);

	int tamanoX = this->belcebu->getTamy();
	coordenadas seek;

	if (this->equipo = ROJO) {
		seek = make_pair(tamanoX-1, 0);
	} else {
		seek = make_pair(1, 0);
	} 
	
	for (int i = 0; i < tamanoX; i++) {
		
		this->belcebu->en_posicion(seek);
	}

}
/*
4. Defina el metodo coordenadas Equipo::buscar bandera contraria() de la clase equipo donde los jugadores se
repartan el tablero y puedan buscar, de manera paralela/simuiltanea, la bandera contraria. En particular, sera de
interes medir los tiempos de busqueda de la bandera contraria utilizando o no el paralelismo mencionado anteriormente.
Para hacerlo pueden utilizar la funcion clock gettime (con la opcion CLOCK REALTIME) de la biblioteca time.h, que
se puede linkear utilizando -lrt.
*/




/*
1) Antes de empezar a mover, esperamos a encontrar la bandera antes de mvoernos o nos movemos de forma aleatoria hasta saber donde esta? los jugadores usan su turno para buscar la bandera?
2) Posicion bandera roja es segunda columna, posicion bandera azul es ultima, deberia ser anteultima?
*/
