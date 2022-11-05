#include "equipo.h"
#include <assert.h>     /* assert */
#include "definiciones.h"
#include <cmath>
#include <chrono>
#include <thread>
#include <unistd.h>


using namespace std;

direccion Equipo::apuntar_a(coordenadas pos1, coordenadas pos2) {
	if (pos2.second > pos1.second) return ARRIBA;
	if (pos2.second < pos1.second) return ABAJO;
	if (pos2.first < pos1.first) return IZQUIERDA;
	else if (pos2.first > pos1.first) return DERECHA;
	return ARRIBA;
	}


void Equipo::jugador(int nro_jugador) {


	if (pos_bandera_contraria == make_pair(-1,-1)) {
		this->busqueda == MULTI_THREAD : buscar_bandera_contraria(nro_jugador) ? buscar_bandera_contraria_single_thread(nro_jugador);
		while(pos_bandera_contraria == make_pair(-1,-1));
		
	}

	while(true) { 

		// detengo a los jugadores para que jueguen de a uno
		m_turno.lock();

		if(this->belcebu->termino_juego()) { // si habia jugadores en el mutex y el que estaba jugando ganó, se termina la partida
			m_turno.unlock();
			this->equipo == ROJO ? this->belcebu->turno_rojo.unlock() : this->belcebu->turno_azul.unlock();
			break;
		}

		switch(this->strat) {
			//SECUENCIAL,RR,SHORTEST,USTEDES
			case(SECUENCIAL): { // AGREGAR BARRERA: DESPUES DE MOVERSE HACE BARRERA.WAIT ASI CADA JUGADOR SE MUEVE SOLO UNA VEZ.
				if (this->cant_jugadores_que_ya_jugaron < this->cant_jugadores) {
					if (!ya_jugo[nro_jugador]) {
						int movio_jugador = this->belcebu->mover_jugador(apuntar_a(this->posiciones[nro_jugador], this->pos_bandera_contraria),nro_jugador);
						if(movio_jugador == 0) {
								this->posiciones[nro_jugador] = this->belcebu->proxima_posicion(this->posiciones[nro_jugador], apuntar_a(posiciones[nro_jugador], this->pos_bandera_contraria)) ;
						}
						this->cant_jugadores_que_ya_jugaron++;
						ya_jugo[nro_jugador] = true;
					}
				} else {
					this->cant_jugadores_que_ya_jugaron = 0; //reinicio los valores
					this->quantum_restante = this->quantum;
					for (auto elem : ya_jugo) {
						elem = false;
					}
					this->belcebu->termino_ronda(this->equipo);
				}
				break;
			}
			
			case(RR): {
				int jugador_a_mover = this->cant_jugadores_que_ya_jugaron % this->cant_jugadores; //si bien esta es la cuenta que se usa cuando el quantum es menor a la cantidad de jugadores, tambien sirve en el otro caso
				if (jugador_a_mover == nro_jugador){ //chequeo si soy el jugador que debe hacer el movimiento
					if (this->quantum <= this->cant_jugadores){ //chequeo en cual de los dos casos estoy
						if(this->cant_jugadores == this->cant_jugadores_que_ya_jugaron){ //ya jugaron todos
							this->cant_jugadores_que_ya_jugaron = 0; //reinicio los valores
							this->quantum_restante = this->quantum;
							this->belcebu->termino_ronda(this->equipo);
						} else {
							int movio_jugador = this->belcebu->mover_jugador(apuntar_a(posiciones[jugador_a_mover], this->pos_bandera_contraria),jugador_a_mover);
							if(movio_jugador == 0) {
								this->posiciones[nro_jugador] = this->belcebu->proxima_posicion(this->posiciones[nro_jugador], apuntar_a(posiciones[nro_jugador], this->pos_bandera_contraria)) ;
							}
							this->cant_jugadores_que_ya_jugaron++;
							this->quantum_restante--;
						}
					} else {
						if(this->quantum_restante == 0){
							this->cant_jugadores_que_ya_jugaron = 0; //reinicio los valores
							this->quantum_restante = this->quantum;
							this->belcebu->termino_ronda(this->equipo);
						} else {
							int movio_jugador = this->belcebu->mover_jugador(apuntar_a(posiciones[jugador_a_mover], this->pos_bandera_contraria),jugador_a_mover);
							if(movio_jugador == 0) {
								this->posiciones[nro_jugador] = this->belcebu->proxima_posicion(this->posiciones[nro_jugador], apuntar_a(posiciones[nro_jugador], this->pos_bandera_contraria)) ;
							}
							this->cant_jugadores_que_ya_jugaron++;
							this->quantum_restante--;
						}
					}
				}
				break;
			}

			case(SHORTEST): {
				//tengo que asegurarme de llamar al de menor dist
				int jugador_cercano = this->jugador_mas_cercano(); 
				if (nro_jugador == jugador_cercano){ 
					int movio_jugador = this->belcebu->mover_jugador(apuntar_a(posiciones[jugador_cercano], this->pos_bandera_contraria), jugador_cercano);
					if(movio_jugador == 0) {
						this->posiciones[nro_jugador] = this->belcebu->proxima_posicion(this->posiciones[nro_jugador], apuntar_a(posiciones[nro_jugador], this->pos_bandera_contraria)) ;
					}
					this->belcebu->termino_ronda(this->equipo);
				} 
				break;
			}

			case(USTEDES): {
				// La idea es hacer un Round robin combinado con shortest, cuando el quantum es mayor a la cantidad de jugadores 
				// movemos primero una vez a cada jugador y con el quantum restante movemos al jugador mas cercano a la bandera
				//
				int movimientos_destinados_a_shortest = this->quantum - this->cant_jugadores;
				int jugador_a_mover = this->cant_jugadores_que_ya_jugaron % this->cant_jugadores; //si bien esta es la cuenta que se usa cuando el quantum es menor a la cantidad de jugadores, tambien sirve en el otro caso
				if (this->quantum <= this->cant_jugadores){ //chequeo en cual de los dos casos estoy
					//muevo a todos al menos una vez ya que quantum < cant_jugadores
					if(this->cant_jugadores == this->cant_jugadores_que_ya_jugaron){
						this->cant_jugadores_que_ya_jugaron = 0; //reinicio los valores
						this->quantum_restante = this->quantum;
						this->belcebu->termino_ronda(this->equipo);
					} else {
						if (jugador_a_mover == nro_jugador){//chequeo si soy el jugador que debe hacer el movimiento
							int movio_jugador = this->belcebu->mover_jugador(apuntar_a(posiciones[jugador_a_mover], this->pos_bandera_contraria),jugador_a_mover);
							if(movio_jugador == 0) {
								this->posiciones[nro_jugador] = this->belcebu->proxima_posicion(this->posiciones[nro_jugador], apuntar_a(posiciones[nro_jugador], this->pos_bandera_contraria)) ;
							}
							this->cant_jugadores_que_ya_jugaron++;
							this->quantum_restante--;
						}
						
					}
				} else {
					//RR + SHORTEST
					if(this->quantum_restante == 0){
						this->cant_jugadores_que_ya_jugaron = 0; //reinicio los valores
						this->quantum_restante = this->quantum;
						this->belcebu->termino_ronda(this->equipo);
					} else {
						if (movimientos_destinados_a_shortest >= this->quantum_restante){ //destino #quantum - #cantidad_de_jugadores movimientos a shortest y el resto a quantum
							//muevo de forma SHORTEST 
							int jugador_cercano = this->jugador_mas_cercano(); 
							if (nro_jugador == jugador_cercano){ //podriamos llegar a implementar un else que duerma a los jugadores que no son el mas cercano, pero complicaria el codigo, puede causar deadlocks y no es mucho mas optimo
								while(this->quantum_restante > 0 && !this->belcebu->termino_juego()) { // lo muevo #quantum - #cantidad_de_jugadores veces
									int movio_jugador = this->belcebu->mover_jugador(apuntar_a(posiciones[jugador_cercano], this->pos_bandera_contraria), jugador_cercano);
									if(movio_jugador == 0) {
										this->posiciones[nro_jugador] = this->belcebu->proxima_posicion(this->posiciones[nro_jugador], apuntar_a(posiciones[nro_jugador], this->pos_bandera_contraria)) ;
									}
									this->quantum_restante--;
									
								}
							} 
						} else {
							if (jugador_a_mover == nro_jugador){//chequeo si soy el jugador que debe hacer el movimiento
								//muevo de forma RR
								int movio_jugador = this->belcebu->mover_jugador(apuntar_a(posiciones[jugador_a_mover], this->pos_bandera_contraria),jugador_a_mover);
								if(movio_jugador == 0) {
									this->posiciones[nro_jugador] = this->belcebu->proxima_posicion(this->posiciones[nro_jugador], apuntar_a(posiciones[nro_jugador], this->pos_bandera_contraria)) ;
								}
								this->cant_jugadores_que_ya_jugaron++;
								this->quantum_restante--;
							}
							
						}
					}
				}
				break;
			}
			default:
				break;
		}	

		if (this->strat != SHORTEST && this->belcebu->termino_juego()){ //si llegué a la bandera termino la ronda porque gané 
			this->belcebu->termino_ronda(this->equipo);
		}
		
		m_turno.unlock();
	}
	
}

Equipo::Equipo(gameMaster *belcebu, color equipo, 
		estrategia strat, tipo_busqueda busqueda, int cant_jugadores, int quantum, vector<coordenadas> posiciones) {
	this->belcebu = belcebu;
	this->equipo = equipo;
	this->contrario = (equipo == ROJO)? AZUL: ROJO;
	this->bandera_contraria = (equipo==ROJO)? BANDERA_AZUL: BANDERA_ROJA;
	this->strat = strat;
	this->busqueda = busqueda;
	this->quantum = quantum;
	this->quantum_restante = quantum;
	this->cant_jugadores = cant_jugadores;
	this->posiciones = posiciones;
	this->pos_bandera_contraria = make_pair(-1,-1);
	vector<bool> vecAux (cant_jugadores,false);
	this->ya_jugo = vecAux;

}

void Equipo::comenzar() {
	// Arranco cuando me toque el turno 
	clock_gettime(CLOCK_REALTIME, &(this->strat_inicio));
	if(this->equipo == AZUL) (this->belcebu->turno_azul).lock(); // Pongo a esperar el azul y cuando termine el rojo va a iniciar este
	
	// Creamos los jugadores
	for(int i=0; i < cant_jugadores; i++) {
		jugadores.emplace_back(thread(&Equipo::jugador, this, i)); 
	}

}

void Equipo::terminar() {
	for(auto &t:jugadores){
		t.join();
	}	
	clock_gettime(CLOCK_REALTIME, &(this->strat_fin));
	this->tiempo_strat.first = abs(this->strat_fin.tv_sec - this->strat_inicio.tv_sec);
	this->tiempo_strat.second = abs(this->strat_fin.tv_nsec - this->strat_inicio.tv_nsec);
	printf("Tiempo total de la estrategia, equipo %i: %i segundos %i nanosegundos\n", this->equipo ,this->tiempo_strat.first, this->tiempo_strat.second);

}

coordenadas Equipo::buscar_bandera_contraria(int nro_jugador) {

	clock_gettime(CLOCK_REALTIME, &(this->busqueda_inicio));

	float tamX = this->belcebu->getTamx();
	float tamY = this->belcebu->getTamy();
	float jugadores = this->cant_jugadores;
	coordenadas coord_bandera;

	float cantColumnas = ceil(tamX / jugadores);
	int columnaInicial = nro_jugador * cantColumnas;
	int columnaFinal = (nro_jugador+1) * cantColumnas;

	if ((columnaFinal >= tamX)) columnaFinal = tamX-1;
	
	assert(columnaInicial >= 0 && columnaFinal < tamX);

	for (int i = columnaInicial; i <= columnaFinal; i++) {
		for (int j = 0; j < tamY; j++) {
			if ((this->equipo == ROJO && this->belcebu->en_posicion(make_pair(i, j)) == BANDERA_AZUL) ||
				(this->equipo == AZUL && this->belcebu->en_posicion(make_pair(i, j)) == BANDERA_ROJA)
				) {
				this->pos_bandera_contraria = make_pair(i, j);
				coord_bandera = make_pair(i,j);
				break;
			}
		}
		if (this->pos_bandera_contraria != make_pair(-1, -1)) {
			break;
		}
	}

	clock_gettime(CLOCK_REALTIME, &(this->busqueda_fin));
	this->tiempo_busqueda.first = abs(this->busqueda_fin.tv_sec - this->busqueda_inicio.tv_sec);
	this->tiempo_busqueda.second = abs(this->busqueda_fin.tv_nsec - this->busqueda_inicio.tv_nsec);
	printf("Tiempo de busqueda de bandera: %i segundos %i nanosegundos\n", this->tiempo_busqueda.first, this->tiempo_busqueda.second);

	return coord_bandera;
}

int Equipo::jugador_mas_cercano() {
	int mas_cercano = 0;
	int distancia_minima = this->belcebu->distancia(this->pos_bandera_contraria, posiciones[0]);
	for (int i = 1; i<cant_jugadores; i++){
		int distancia = this->belcebu->distancia(this->pos_bandera_contraria, posiciones[i]);
		if (distancia<distancia_minima){
			mas_cercano = i;
			distancia_minima = distancia;
		}
	}
	return mas_cercano;
}

coordenadas Equipo::buscar_bandera_contraria_single_thread() {

	clock_gettime(CLOCK_REALTIME, &(this->busqueda_inicio));

	int tamX = this->belcebu->getTamx();
	int tamY = this->belcebu->getTamy();
	int columnaInicial;
	coordenadas coord_bandera;
	if (this->equipo == ROJO) {
		columnaInicial = tamX-1;
		for (int i = columnaInicial; i >= 0; i--) {
			for (int j = 0; j < tamY; j++) {
				if (this->belcebu->en_posicion(make_pair(i, j)) == BANDERA_AZUL) {
					this->pos_bandera_contraria = make_pair(i, j);
					coord_bandera = make_pair(i,j);
					break;
				}
			}
			if (this->pos_bandera_contraria != make_pair(-1, -1)) {
				break;
			}
		}
	} else {
		columnaInicial = 0;
		for (int i = columnaInicial; i < tamX; i++) {
			for (int j = 0; j < tamY; j++) {
				if (this->belcebu->en_posicion(make_pair(i, j)) == BANDERA_ROJA) {
					coord_bandera = make_pair(i,j);
					break;
				}
			}
			if (this->pos_bandera_contraria != make_pair(-1, -1)) {
				break;
			}
		}
	}

	clock_gettime(CLOCK_REALTIME, &(this->busqueda_fin));
	this->tiempo_busqueda.first = abs(this->busqueda_fin.tv_sec - this->busqueda_inicio.tv_sec);
	this->tiempo_busqueda.second = abs(this->busqueda_fin.tv_nsec - this->busqueda_inicio.tv_nsec);
	printf("Tiempo de busqueda de bandera: %i segundos %i nanosegundos\n", this->tiempo_busqueda.first, this->tiempo_busqueda.second);

	return coord_bandera;
}

