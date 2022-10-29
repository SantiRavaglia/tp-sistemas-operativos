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
	while(pos_bandera_contraria == make_pair(-1,-1)){
		int tamX = this->belcebu->getTamx();
		int tamY = this->belcebu->getTamy();
		//Que hacer si la division del tablero da un numero con coma?
		//Redondeo para arriba y algunos jugadores revisas la misma casilla a veces
		int cantCasillas = (int)ceil((float)(tamX * tamY)/(float)this->cant_jugadores);
		int casillaInicio = cantCasillas * nro_jugador;
		// Si algun jugador se pasa del limite del tablero le asignamos tamaño - 1 a su limite
		if (casillaInicio+cantCasillas >= tamX*tamY){
			cantCasillas = tamX*tamY - casillaInicio - 1;
		}
		buscar_bandera_contraria(casillaInicio, cantCasillas);
	}
	while(true) { // Chequear que no haya una race condition en gameMaster

	m_turno.lock();

	if(this->belcebu->termino_juego()) {
		m_turno.unlock();
		this->equipo == ROJO ? this->belcebu->turno_rojo.unlock() : this->belcebu->turno_azul.unlock();
		break;
	}
		switch(this->strat) {
			//SECUENCIAL,RR,SHORTEST,USTEDES
			case(SECUENCIAL): { // AGREGAR BARRERA: DESPUES DE MOVERSE HACE BARRERA.WAIT ASI CADA JUGADOR SE MUEVE SOLO UNA VEZ.
				printf("jugador nro %i, jugo? -> %i, ya jugaron %i jugadores\n", nro_jugador, ya_jugo[nro_jugador] == true ? 1 : 0, this->cant_jugadores_que_ya_jugaron);
				if (this->cant_jugadores_que_ya_jugaron < this->cant_jugadores) {
					if (!ya_jugo[nro_jugador]) {
						this->belcebu->mover_jugador(apuntar_a(this->posiciones[nro_jugador], this->pos_bandera_contraria),nro_jugador);
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
				printf("quantum = %i, ya jugaron %i jugadores \n", this->quantum_restante, this->cant_jugadores_que_ya_jugaron);
					if (this->quantum <= this->cant_jugadores){ //chequeo en cual de los dos casos estoy
						if(this->cant_jugadores == this->cant_jugadores_que_ya_jugaron){ //ya jugaron todos
							this->cant_jugadores_que_ya_jugaron = 0; //reinicio los valores
							this->quantum_restante = this->quantum;
							this->belcebu->termino_ronda(this->equipo);
						} else {
							this->belcebu->mover_jugador(apuntar_a(posiciones[jugador_a_mover], this->pos_bandera_contraria),jugador_a_mover);
							this->cant_jugadores_que_ya_jugaron++;
							this->quantum_restante--;
						}
					} else {
						if(this->quantum_restante == 0){
							this->cant_jugadores_que_ya_jugaron = 0; //reinicio los valores
							this->quantum_restante = this->quantum;
							this->belcebu->termino_ronda(this->equipo);
						} else {
							this->belcebu->mover_jugador(apuntar_a(posiciones[jugador_a_mover], this->pos_bandera_contraria),jugador_a_mover);
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
				printf("------------ JUGADOR NUEVO ------------ arranca jugador numero %i del equipo %i a moverse en la posicion (%i, %i) direccion %i \n", nro_jugador, this->equipo, this->posiciones[nro_jugador].first, this->posiciones[nro_jugador].second, apuntar_a(posiciones[nro_jugador], this->pos_bandera_contraria));
				int jugador_cercano = this->jugador_mas_cercano(); 
				if (nro_jugador == jugador_cercano){ 
					printf("entra el jugador mas cercano %i del equipo %i al if\n", nro_jugador, this->equipo);
					int movio_jugador = this->belcebu->mover_jugador(apuntar_a(posiciones[jugador_cercano], this->pos_bandera_contraria), jugador_cercano);
					if(movio_jugador == 0) {
						this->posiciones[nro_jugador] = this->belcebu->proxima_posicion(this->posiciones[nro_jugador], apuntar_a(posiciones[nro_jugador], this->pos_bandera_contraria)) ;
						printf("jugador ahora en la posicion (%i, %i)\n", this->posiciones[nro_jugador].first, this->posiciones[nro_jugador].second);
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
				//printf("\n nro jugador %i, jugador a mover %i - ", nro_jugador, jugador_a_mover);
				if (jugador_a_mover == nro_jugador){ //chequeo si soy el jugador que debe hacer el movimiento
					//printf("quantum = %i, ya jugaron %i jugadores, movimientos para shortest: %i", this->quantum_restante, this->cant_jugadores_que_ya_jugaron, movimientos_destinados_a_shortest);
					if (this->quantum <= this->cant_jugadores){ //chequeo en cual de los dos casos estoy
						if(this->cant_jugadores == this->cant_jugadores_que_ya_jugaron){
							this->cant_jugadores_que_ya_jugaron = 0; //reinicio los valores
							this->quantum_restante = this->quantum;
							this->belcebu->termino_ronda(this->equipo);
						} else {
							this->belcebu->mover_jugador(apuntar_a(posiciones[jugador_a_mover], this->pos_bandera_contraria),jugador_a_mover);
							this->cant_jugadores_que_ya_jugaron++;
							this->quantum_restante--;
						}
					} else {
						if(this->quantum_restante == 0){
							this->cant_jugadores_que_ya_jugaron = 0; //reinicio los valores
							this->quantum_restante = this->quantum;
							this->belcebu->termino_ronda(this->equipo);
						} else {
							if (movimientos_destinados_a_shortest >= this->quantum_restante){ //destino #quantum - #cantidad_de_jugadores movimientos a shortest y el resto a quantum
								printf(" - muevo de al mas cercano\n");
								int jugador_cercano = this->jugador_mas_cercano(); 
								if (nro_jugador == jugador_cercano){ //podriamos llegar a implementar un else que duerma a los jugadores que no son el mas cercano, pero complicaria el codigo, puede causar deadlocks y no es mucho mas optimo
									while(this->quantum_restante > 0 && this->belcebu->ganador == INDEFINIDO) { // lo muevo #quantum - #cantidad_de_jugadores veces
										this->belcebu->mover_jugador(apuntar_a(posiciones[jugador_cercano], this->pos_bandera_contraria), jugador_cercano);
										printf("jugador ahora en la posicion (%i, %i)\n", this->posiciones[nro_jugador].first, this->posiciones[nro_jugador].second);
										this->quantum_restante--;
										
									}

									/*this->cant_jugadores_que_ya_jugaron = 0; //reinicio los valores
									this->quantum_restante = this->quantum;
									this->belcebu->termino_ronda(this->equipo);*/
								} 
							} else {
								//printf(" - muevo de forma secuencial\n");
								this->belcebu->mover_jugador(apuntar_a(posiciones[jugador_a_mover], this->pos_bandera_contraria),jugador_a_mover);
								printf("jugador ahora en la posicion (%i, %i)\n", this->posiciones[nro_jugador].first, this->posiciones[nro_jugador].second);
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
		
		if (this->belcebu->termino_juego()){ //si llegué a la bandera termino la ronda porque gané 
			this->belcebu->termino_ronda(this->equipo);
		}

		//this_thread::sleep_for(1000ms);
		m_turno.unlock();
		//this_thread::sleep_for(500ms);
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
	this->pos_bandera_contraria = make_pair(-1,-1);
	vector<bool> vecAux (cant_jugadores,false);
	this->ya_jugo = vecAux;

	//
	// ...
	//

	// if (strat == SHORTEST) {
		this->buscar_bandera_contraria_single_thread();
	// }
	printf("bandera contraria: (%i , %i)\n", this->pos_bandera_contraria.first, this->pos_bandera_contraria.second);
}

void Equipo::comenzar() {
	// Arranco cuando me toque el turno 
	// TODO: Quien empieza ? No se si lo de abajo esta bien
	//fsem_wait(&(belcebu->turno_rojo)); // Inicializo el rojo
	if(this->equipo == AZUL) (this->belcebu->turno_azul).lock(); // Pongo a esperar el azul y cuando termine el rojo va a iniciar este
	// Creamos los jugadores

	// barrier barrera_equipo(this->cant_jugadores);

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
	coordenadas coord_bandera;
	for (int i = filaInicial; i <= filaFinal; i++){
		// Tengo que llamar en la primera iteracion desde la columna en la que empiece
		// y en la ultima iteracion de la columna en la que termino
		int inicio = 0;
		int fin = tamX - 1;
		if (i == filaFinal) fin = columnaFinal; // Ultima fila solo iteramos hasta la columnaFinal del jugador
		if (primeraIteracion) inicio = columnaInicial;
		for (int j = inicio ; j <= fin; j++){
			if(this->belcebu->en_posicion(make_pair(i,j)) == bandera_contraria){
				this->pos_bandera_contraria = make_pair(i,j);
				coord_bandera = make_pair(i,j);
			};
		}
		primeraIteracion = false;
	}
	/*
	FALTA FIJARSE:
	- Si puedo asignar la bandera del equipo contraria asi nomas o tengo que devolverla al jugador y eso pasarla al belcebu
	- Agregar algun flag para cortar la iteracion en todos los demas jugadores?
	*/
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

	int tamX = this->belcebu->getTamx();
	int tamY = this->belcebu->getTamy();
	int columnaInicial;
	coordenadas coord_bandera;
	if (this->equipo == ROJO) {
		columnaInicial = tamX-1;
		for (int i = columnaInicial; i >= 0; i--) {
			for (int j = 0; j < tamY; j++) {
				printf("6, (%i , %i) \n", i, j);
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
					this->pos_bandera_contraria = make_pair(i, j);
					coord_bandera = make_pair(i,j);
					break;
				}
			}
			if (this->pos_bandera_contraria != make_pair(-1, -1)) {
				break;
			}
		}
	}
	return coord_bandera;
}





// coordenadas Equipo::buscar_bandera_contraria(int numJugador) { FALTA DESARROLLAR
// 	//
// 	// ...
// 	//

// 	assert(this->equipo == ROJO && this->equipo == AZUL);

// 	int tamanoX = this->belcebu->getTamy();
// 	coordenadas seek;

// 	int cantColumnas = ceil(tamanoX / this->cant_jugadores);
// 	pair<int,int> rangoBusqueda = make_pair(cantColumnas*numJugador, cantColumnas*(numJugador+1)-1);

// 	if (this->equipo = ROJO) {
// 		seek = make_pair(tamanoX-1, 0);
// 	} else {
// 		seek = make_pair(1, 0);
// 	} 
	
// 	for (int i = 0; i < tamanoX; i++) {
		
// 		this->belcebu->en_posicion(seek);
// 	}

// }


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
