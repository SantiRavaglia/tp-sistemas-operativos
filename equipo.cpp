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
	while(pos_bandera_contraria == make_pair(-1,-1)){
		int tamX = this->belcebu->getTamx();
		int tamY = this->belcebu->getTamy();
		int cantCasillas = (tamX * tamY)/cant_jugadores;
		int casillaInicio = cantCasillas * nro_jugador;
		// Que hacer si la division del tablero da un numero con coma?
		buscar_bandera_contraria(casillaInicio, cantCasillas);
		
		// Dividir el tablero
		
		// Mandar a buscar la bandera

	}




	while(!this->belcebu->termino_juego()) { // Chequear que no haya una race condition en gameMaster
		switch(this->strat) {
			//SECUENCIAL,RR,SHORTEST,USTEDES
			case(SECUENCIAL):
				this->belcebu->m_turno.lock();
				if (cant_jugadores_que_ya_jugaron == cant_jugadores) {
					this->belcebu->termino_ronda(this->equipo); //HAY QUE DESBLOQUEAR EL MUTEX
					// Hay que refreshear el quantum/jugadores que ya jugaron al principio de la ronda
				} else {
					this->belcebu->mover_jugador(apuntar_a(posiciones[nro_jugador], pos_bandera_contraria),nro_jugador);
					cant_jugadores_que_ya_jugaron++;
				}
				this->belcebu->m_turno.unlock();
				break;
			
			case(RR):
			// Quizas vaya aca el semaforo y la resta de quantum donde va?
				this->belcebu->m_turno.lock();
				if(quantum_restante == 0){
					quantum_restante = quantum;
					this->belcebu->termino_ronda(this->equipo);
				} else {
					int jugador_a_mover = cant_jugadores_que_ya_jugaron % cant_jugadores;
					this->belcebu->mover_jugador(apuntar_a(posiciones[jugador_a_mover], pos_bandera_contraria),jugador_a_mover);
					cant_jugadores_que_ya_jugaron++;
					quantum_restante--;
				}
				this->belcebu->m_turno.unlock();
				break;

			case(SHORTEST):
			// 	Tengo que ver de donde saco la dist a la bandera
			// Si es un movimiento random o hacia la bandera 
			// Si esta es la strat tengo que asegurarme de llamar al de menor dist
				this->belcebu->mover_jugador("DIRECCION", nro_jugador);
				this->belcebu->termino_ronda(this->equipo);
				break;

			case(USTEDES):
				// La idea es hacer un Round robin combinado con shortest, cuando el quantum es mayor a la cantidad de jugadores 
				// movemos primero una vez a cada jugador y con el quantum restante movemos al jugador mas cercano a la bandera
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

coordenadas Equipo::buscar_bandera_contraria(int casillaInicio, int cantCasillas) {
	int tamX = this->belcebu->getTamx();
	int tamY = this->belcebu->getTamy();
	int filaInicial = casillaInicio/tamX;
	int columnaInicial = casillaInicio % tamX;
	int casillaFinal = casillaInicio + cantCasillas;
	int filaFinal = casillaInicio/tamX;
	int columnaFinal = casillaInicio % tamX;
	bool primeraIteracion = true;
	for (int i = filaInicial; i <= filaFinal; i++){
		// Tengo que llamar en la primera iteracion desde la columna en la que empiece
		// y en la ultima iteracion de la columna en la que termino
		int inicio = 0;
		int fin = tamX - 1;
		if (i == filaFinal) fin = columnaFinal; // Ultima fila solo iteramos hasta la columnaFinal del jugador
		if (primeraIteracion) inicio = columnaInicial;
		for (int j = inicio ; j <= fin; j++){
			if(this->belcebu->en_posicion(make_pair(i,j)) == bandera_contraria){
				pos_bandera_contraria = make_pair(i,j);
			};
		}
		primeraIteracion = false;
	}
	/*
	FALTA FIJARSE:
	- Si puedo asignar la bandera del equipo contraria asi nomas o tengo que devolverla al jugador y eso pasarla al belcebu
	- Agregar algun flag para cortar la iteracion en todos los demas jugadores?
	*/
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
