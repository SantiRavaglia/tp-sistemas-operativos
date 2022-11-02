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
	// while(pos_bandera_contraria == make_pair(-1,-1)){
	// 	int tamX = this->belcebu->getTamx();
	// 	int tamY = this->belcebu->getTamy();
	// 	//Que hacer si la division del tablero da un numero con coma?
	// 	//Redondeo para arriba y algunos jugadores revisas la misma casilla a veces
	// 	int cantCasillas = (int)ceil((float)(tamX * tamY)/(float)this->cant_jugadores);
	// 	int casillaInicio = cantCasillas * nro_jugador;
	// 	// Si algun jugador se pasa del limite del tablero le asignamos tamaño - 1 a su limite
	// 	if (casillaInicio+cantCasillas >= tamX*tamY){
	// 		cantCasillas = tamX*tamY - casillaInicio - 1;
	// 	}
	// 	buscar_bandera_contraria(casillaInicio, cantCasillas);
	// }


	if (pos_bandera_contraria == make_pair(-1,-1)) {
		//printf("equipo %i\n", this->equipo);
		buscar_bandera_contraria(nro_jugador);
		while(pos_bandera_contraria == make_pair(-1,-1));
		
	}

	while(true) { 

		m_turno.lock();
		// printf("ENTRA A JUGAR JUGADOR %i", nro_jugador);

		if(this->belcebu->termino_juego()) {
			m_turno.unlock();
			this->equipo == ROJO ? this->belcebu->turno_rojo.unlock() : this->belcebu->turno_azul.unlock();
			break;
		}
		switch(this->strat) {
			//SECUENCIAL,RR,SHORTEST,USTEDES
			case(SECUENCIAL): { // AGREGAR BARRERA: DESPUES DE MOVERSE HACE BARRERA.WAIT ASI CADA JUGADOR SE MUEVE SOLO UNA VEZ.
				// printf("jugador nro %i, jugo? -> %i, ya jugaron %i jugadores\n", nro_jugador, ya_jugo[nro_jugador] == true ? 1 : 0, this->cant_jugadores_que_ya_jugaron);
				if (this->cant_jugadores_que_ya_jugaron < this->cant_jugadores) {
					if (!ya_jugo[nro_jugador]) {
						int movio_jugador = this->belcebu->mover_jugador(apuntar_a(this->posiciones[nro_jugador], this->pos_bandera_contraria),nro_jugador);
						if(movio_jugador == 0) {
								this->posiciones[nro_jugador] = this->belcebu->proxima_posicion(this->posiciones[nro_jugador], apuntar_a(posiciones[nro_jugador], this->pos_bandera_contraria)) ;
								// printf("jugador ahora en la posicion (%i, %i)\n", this->posiciones[nro_jugador].first, this->posiciones[nro_jugador].second);
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
					// Hay que refreshear el quantum/jugadores que ya jugaron al principio de la ronda
				}
				break;
			}
			
			case(RR): {
				int jugador_a_mover = this->cant_jugadores_que_ya_jugaron % this->cant_jugadores; //si bien esta es la cuenta que se usa cuando el quantum es menor a la cantidad de jugadores, tambien sirve en el otro caso
				if (jugador_a_mover == nro_jugador){ //chequeo si soy el jugador que debe hacer el movimiento
				// printf("quantum = %i, ya jugaron %i jugadores \n", this->quantum_restante, this->cant_jugadores_que_ya_jugaron);
					if (this->quantum <= this->cant_jugadores){ //chequeo en cual de los dos casos estoy
						if(this->cant_jugadores == this->cant_jugadores_que_ya_jugaron){ //ya jugaron todos
							this->cant_jugadores_que_ya_jugaron = 0; //reinicio los valores
							this->quantum_restante = this->quantum;
							this->belcebu->termino_ronda(this->equipo);
						} else {
							int movio_jugador = this->belcebu->mover_jugador(apuntar_a(posiciones[jugador_a_mover], this->pos_bandera_contraria),jugador_a_mover);
							if(movio_jugador == 0) {
								this->posiciones[nro_jugador] = this->belcebu->proxima_posicion(this->posiciones[nro_jugador], apuntar_a(posiciones[nro_jugador], this->pos_bandera_contraria)) ;
								// printf("jugador ahora en la posicion (%i, %i)\n", this->posiciones[nro_jugador].first, this->posiciones[nro_jugador].second);
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
								// printf("jugador ahora en la posicion (%i, %i)\n", this->posiciones[nro_jugador].first, this->posiciones[nro_jugador].second);
							}
							this->cant_jugadores_que_ya_jugaron++;
							this->quantum_restante--;
						}
					}
				}
				break;
			}

			case(SHORTEST): {
			// 	Tengo que ver de donde saco la dist a la bandera
			// Si esta es la strat tengo que asegurarme de llamar al de menor dist
				// printf("------------ JUGADOR NUEVO ------------ arranca jugador numero %i del equipo %i a moverse en la posicion (%i, %i) direccion %i \n", nro_jugador, this->equipo, this->posiciones[nro_jugador].first, this->posiciones[nro_jugador].second, apuntar_a(posiciones[nro_jugador], this->pos_bandera_contraria));
				int jugador_cercano = this->jugador_mas_cercano(); 
				if (nro_jugador == jugador_cercano){ 
					// printf("entra el jugador mas cercano %i del equipo %i al if\n", nro_jugador, this->equipo);
					//printf("%i - ", this->equipo);
					int movio_jugador = this->belcebu->mover_jugador(apuntar_a(posiciones[jugador_cercano], this->pos_bandera_contraria), jugador_cercano);
					if(movio_jugador == 0) {
						this->posiciones[nro_jugador] = this->belcebu->proxima_posicion(this->posiciones[nro_jugador], apuntar_a(posiciones[nro_jugador], this->pos_bandera_contraria)) ;
						// printf("jugador ahora en la posicion (%i, %i)\n", this->posiciones[nro_jugador].first, this->posiciones[nro_jugador].second);
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
					//muevo a todos al menos una vez
					if(this->cant_jugadores == this->cant_jugadores_que_ya_jugaron){
						this->cant_jugadores_que_ya_jugaron = 0; //reinicio los valores
						this->quantum_restante = this->quantum;
						this->belcebu->termino_ronda(this->equipo);
					} else {
						if (jugador_a_mover == nro_jugador){//chequeo si soy el jugador que debe hacer el movimiento
							int movio_jugador = this->belcebu->mover_jugador(apuntar_a(posiciones[jugador_a_mover], this->pos_bandera_contraria),jugador_a_mover);
							if(movio_jugador == 0) {
								this->posiciones[nro_jugador] = this->belcebu->proxima_posicion(this->posiciones[nro_jugador], apuntar_a(posiciones[nro_jugador], this->pos_bandera_contraria)) ;
								// printf("jugador ahora en la posicion (%i, %i)\n", this->posiciones[nro_jugador].first, this->posiciones[nro_jugador].second);
							}
							this->cant_jugadores_que_ya_jugaron++;
							this->quantum_restante--;
						}
						
					}
				} else {
					//round robin
					if(this->quantum_restante == 0){
						this->cant_jugadores_que_ya_jugaron = 0; //reinicio los valores
						this->quantum_restante = this->quantum;
						this->belcebu->termino_ronda(this->equipo);
					} else {
						if (movimientos_destinados_a_shortest >= this->quantum_restante){ //destino #quantum - #cantidad_de_jugadores movimientos a shortest y el resto a quantum
							// printf(" - muevo de al mas cercano\n");
							int jugador_cercano = this->jugador_mas_cercano(); 
							if (nro_jugador == jugador_cercano){ //podriamos llegar a implementar un else que duerma a los jugadores que no son el mas cercano, pero complicaria el codigo, puede causar deadlocks y no es mucho mas optimo
								while(this->quantum_restante > 0 && !this->belcebu->termino_juego()) { // lo muevo #quantum - #cantidad_de_jugadores veces
									int movio_jugador = this->belcebu->mover_jugador(apuntar_a(posiciones[jugador_cercano], this->pos_bandera_contraria), jugador_cercano);
									if(movio_jugador == 0) {
										this->posiciones[nro_jugador] = this->belcebu->proxima_posicion(this->posiciones[nro_jugador], apuntar_a(posiciones[nro_jugador], this->pos_bandera_contraria)) ;
										// printf("jugador ahora en la posicion (%i, %i)\n", this->posiciones[nro_jugador].first, this->posiciones[nro_jugador].second);
									}
									// printf("jugador ahora en la posicion (%i, %i)\n", this->posiciones[nro_jugador].first, this->posiciones[nro_jugador].second);
									this->quantum_restante--;
									
								}
							} 
						} else {
							if (jugador_a_mover == nro_jugador){//chequeo si soy el jugador que debe hacer el movimiento
								//printf(" - muevo de forma secuencial\n");
								int movio_jugador = this->belcebu->mover_jugador(apuntar_a(posiciones[jugador_a_mover], this->pos_bandera_contraria),jugador_a_mover);
								if(movio_jugador == 0) {
									this->posiciones[nro_jugador] = this->belcebu->proxima_posicion(this->posiciones[nro_jugador], apuntar_a(posiciones[nro_jugador], this->pos_bandera_contraria)) ;
									// printf("jugador ahora en la posicion (%i, %i)\n", this->posiciones[nro_jugador].first, this->posiciones[nro_jugador].second);
								}
								// printf("jugador ahora en la posicion (%i, %i)\n", this->posiciones[nro_jugador].first, this->posiciones[nro_jugador].second);
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
	this->pos_bandera_contraria = make_pair(-1,-1);
	vector<bool> vecAux (cant_jugadores,false);
	this->ya_jugo = vecAux;


	if (strat == SHORTEST) { 
		this->pos_bandera_contraria = this->buscar_bandera_contraria_single_thread();
		printf("bandera contraria: (%i, %i)\n", this->pos_bandera_contraria.first, this->pos_bandera_contraria.second);
	}
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
	this->tiempo_strat.second = this->tiempo_strat.second/1000000;
	/*int Ini = this->busqueda_inicio.tv_sec;
	int IniN = this->busqueda_inicio.tv_nsec;
	int Fin = this->busqueda_fin.tv_sec;
	int FinN = this->busqueda_fin.tv_nsec;*/
	printf("Tiempo total de la estrategia, equipo %i: %i segundos %i milisegundos\n", this->equipo ,this->tiempo_strat.first, this->tiempo_strat.second);

}

coordenadas Equipo::buscar_bandera_contraria(int nro_jugador) {

	printf("Soy equipo %i", this->equipo);
	clock_gettime(CLOCK_REALTIME, &(this->busqueda_inicio));

	int tamX = this->belcebu->getTamx();
	int tamY = this->belcebu->getTamy();
	coordenadas coord_bandera;

	int cantColumnas = ceil(tamX / this->cant_jugadores);

	int columnaInicial = nro_jugador * cantColumnas;
	int columnaFinal = (nro_jugador+1) * cantColumnas;

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
	/*int auxIni = this->busqueda_inicio.tv_sec;
	int auxIniN = this->busqueda_inicio.tv_nsec;
	int auxFin = this->busqueda_fin.tv_sec;
	int auxFinN = this->busqueda_fin.tv_nsec;*/
	this->tiempo_busqueda.second = this->tiempo_busqueda.second/1000000;
	//printf("Tiempo de busqueda de bandera: %i segundos %i nanosegundos, inicio: %i - %i , fin: %i - %i \n", this->tiempo_busqueda.first, this->tiempo_busqueda.second, auxIni, auxIniN, auxFin, auxFinN);		//printf("Tiempo de busqueda de bandera: %i segundos %i nanosegundos, inicio: %i - %i , fin: %i - %i \n", this->tiempo_busqueda.first, this->tiempo_busqueda.second, auxIni, auxIniN, auxFin, auxFinN);
	printf("Tiempo de busqueda de bandera: %i segundos %i milisegundos\n", this->tiempo_busqueda.first, this->tiempo_busqueda.second);
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
	/*int auxIni = this->busqueda_inicio.tv_sec;
	int auxIniN = this->busqueda_inicio.tv_nsec;
	int auxFin = this->busqueda_fin.tv_sec;
	int auxFinN = this->busqueda_fin.tv_nsec;*/
	this->tiempo_busqueda.second = this->tiempo_busqueda.second/1000000;
	//printf("Tiempo de busqueda de bandera: %i segundos %i nanosegundos, inicio: %i - %i , fin: %i - %i \n", this->tiempo_busqueda.first, this->tiempo_busqueda.second, auxIni, auxIniN, auxFin, auxFinN);
	printf("Tiempo de busqueda de bandera: %i segundos %i milisegundos\n", this->tiempo_busqueda.first, this->tiempo_busqueda.second);
	return coord_bandera;
}




// 8 8 2
// 1 7
// 7 0
// 0 0 0 4 
// 7 7 5 3 

/*
coordenadas Equipo::buscar_bandera_contraria(int casillaInicio, int cantCasillas) {

	clock_gettime(CLOCK_REALTIME, &(this->busqueda_inicio));

	int tamX = this->belcebu->getTamx();
	int tamY = this->belcebu->getTamy();
	int filaInicial = casillaInicio/tamX;
	int columnaInicial = casillaInicio % tamX;
	int casillaFinal = casillaInicio + cantCasillas;
	int filaFinal = casillaFinal/tamX;
	int columnaFinal = casillaFinal % tamX;
	bool primeraIteracion = true;
	coordenadas coord_bandera;
	for (int i = filaInicial; i <= filaFinal; i++){
		// Tengo que llamar en la primera iteracion desde la columna en la que empiece
		// y en la ultima iteracion de la columna en la que termino
		int inicio = 0;
		int fin = tamY - 1;
		if (i == filaFinal) fin = columnaFinal; // Ultima fila solo iteramos hasta la columnaFinal del jugador
		if (primeraIteracion) inicio = columnaInicial;
		for (int j = inicio ; j <= fin; j++){
			if(this->belcebu->en_posicion(make_pair(i,j)) == bandera_contraria){
				this->pos_bandera_contraria = make_pair(i,j);
				coord_bandera = make_pair(i,j);
				printf("bandera contraria: (%i, %i)\n", coord_bandera.first, coord_bandera.second);
			}
		}
		primeraIteracion = false;
	}
	/*
	FALTA FIJARSE:
	- Si puedo asignar la bandera del equipo contraria asi nomas o tengo que devolverla al jugador y eso pasarla al belcebu
	- Agregar algun flag para cortar la iteracion en todos los demas jugadores?
	

	clock_gettime(CLOCK_REALTIME, &(this->busqueda_fin));
	this->tiempo_busqueda.first = abs(this->busqueda_fin.tv_sec - this->busqueda_inicio.tv_sec);
	this->tiempo_busqueda.second = abs(this->busqueda_fin.tv_nsec - this->busqueda_inicio.tv_nsec);
	int auxIni = this->busqueda_inicio.tv_sec;
	int auxIniN = this->busqueda_inicio.tv_nsec;
	int auxFin = this->busqueda_fin.tv_sec;
	int auxFinN = this->busqueda_fin.tv_nsec;
	printf("Tiempo de busqueda de bandera: %i segundos %i nanosegundos, inicio: %i - %i , fin: %i - %i \n", this->tiempo_busqueda.first, this->tiempo_busqueda.second, auxIni, auxIniN, auxFin, auxFinN);

	return coord_bandera;
}*/
 