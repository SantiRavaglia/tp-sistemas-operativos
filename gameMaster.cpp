#include <sys/unistd.h>
#include <assert.h>     /* assert */
#include "gameMaster.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

// SE FIJA SI LA COORDENADA ESTA DENTRO DEL TABLERO
bool gameMaster::es_posicion_valida(coordenadas pos) {
	//cout << "Coordenadas" << pos.first << pos.second <<  endl;
	return (pos.first > -1) && (pos.first < x) && (pos.second > -1) && (pos.second < y);
}

// SE FIJA SI EL TIPO DE CASILLA ES VALIDA PARA MOVERSE
bool gameMaster::es_color_libre(color color_tablero){
    return color_tablero == VACIO || color_tablero == INDEFINIDO;
}

// SE FIJA QUE TIPO DE CASILLA ES LA COORDENADA PROVISTA (JUGADOR, VACIO, BANDERA, ETC)
color gameMaster::en_posicion(coordenadas coord) {
	return tablero[coord.first][coord.second];
}

// TAMAÑO X TABLERO
int gameMaster::getTamx() {
	return x;
}

// TAMAÑO Y TABLERO
int gameMaster::getTamy() {
	return y;
}

// DISTANCIA ENTRE DOS COORDENADAS
int gameMaster::distancia(coordenadas c1, coordenadas c2) {
    return abs(c1.first-c2.first)+abs(c1.second-c2.second);
}

// CONSTRUCTOR GAMEMASTER
gameMaster::gameMaster(Config config) {
	assert(config.x>0); 
	assert(config.y>0); // Tamaño adecuado del tablero

    this->x = config.x;
	this->y = config.y;

	assert((config.bandera_roja.first == 1)); // Bandera roja en la primera columna
	assert(es_posicion_valida(config.bandera_roja)); // Bandera roja en algún lugar razonable

	assert((config.bandera_azul.first == x-1)); // Bandera azul en la ultima columna
	assert(es_posicion_valida(config.bandera_azul)); // Bandera roja en algún lugar razonable

	assert(config.pos_rojo.size() == config.cantidad_jugadores); // JUGADORES EQUIPO ROJO = JUGADORES POR EQUIPO
	assert(config.pos_azul.size() == config.cantidad_jugadores); // JUGADORES EQUIPO AZUL = JUGADORES POR EQUIPO
	for(auto &coord : config.pos_rojo) {
		assert(es_posicion_valida(coord)); // Posiciones validas rojas
	}

	for(auto &coord : config.pos_azul) {
		assert(es_posicion_valida(coord)); // Posiciones validas azul
	}

	
	this->jugadores_por_equipos = config.cantidad_jugadores;
	this->pos_bandera_roja = config.bandera_roja;
	this->pos_bandera_azul = config.bandera_azul;
    this->pos_jugadores_rojos = config.pos_rojo;
    this->pos_jugadores_azules = config.pos_azul;
	// Seteo tablero
	tablero.resize(x);
    for (int i = 0; i < x; ++i) {
        tablero[i].resize(y);
        fill(tablero[i].begin(), tablero[i].end(), VACIO);
    }
    

    for(auto &coord : config.pos_rojo){
        assert(es_color_libre(tablero[coord.first][coord.second])); //Compruebo que no haya otro jugador en esa posicion
        tablero[coord.first][coord.second] = ROJO; // guardo la posicion
    }

    for(auto &coord : config.pos_azul){
        assert(es_color_libre(tablero[coord.first][coord.second]));
        tablero[coord.first][coord.second] = AZUL;
    }

    tablero[config.bandera_roja.first][config.bandera_roja.second] = BANDERA_ROJA;
    tablero[config.bandera_azul.first][config.bandera_azul.second] = BANDERA_AZUL;
	color turno_init = ROJO;
	this->turno = turno_init;
	// sem_init(&turno_rojo,0,1); //inicializo los semaforos, arranca el rojo

	this->turno_azul.lock();
	this->turno_rojo.lock();

    cout << "SE HA INICIALIZADO GAMEMASTER CON EXITO" << endl;
	printf("turno inicial: %i \n", turno_init);
    // Insertar código que crea necesario de inicialización 
}

// MUEVE JUGADOR 
void gameMaster::mover_jugador_tablero(coordenadas pos_anterior, coordenadas pos_nueva, color colorEquipo, int nro_jugador){
    // assert(es_color_libre(tablero[pos_nueva.first][pos_nueva.second]));
    tablero[pos_anterior.first][pos_anterior.second] = VACIO; 
    tablero[pos_nueva.first][pos_nueva.second] = colorEquipo;
	if (turno == ROJO) {
		this->pos_jugadores_rojos[nro_jugador] = pos_nueva;
	} else {
		this->pos_jugadores_azules[nro_jugador] = pos_nueva;
	}
}


int gameMaster::mover_jugador(direccion dir, int nro_jugador) {
	// Chequear que la movida sea valida
	int movimiento_valido = 0;
	coordenadas posicion_jugador;
	if (turno == ROJO) {
		posicion_jugador = this->pos_jugadores_rojos[nro_jugador];
	} else 	{
		posicion_jugador = this->pos_jugadores_azules[nro_jugador];
	}

	printf("proxima posicion: jug n° %i equipo %i en pos (%i, %i) , dir %i \n", nro_jugador, this->turno, posicion_jugador.first, posicion_jugador.second, dir);
	
	if (this->turno == ROJO && (en_posicion(proxima_posicion(posicion_jugador, dir)) == BANDERA_AZUL || es_color_libre(en_posicion(proxima_posicion(posicion_jugador, dir))))) {
		printf("MUEVO JUGADOR\n");
		// SI ES TURNO DEL ROJO Y EN LA COORDENADA A LA QUE SE QUIERE MOVER EL JUGADOR ES COLOR LIBRE O HAY BANDERA AZUL, ME MUEVO
		if (en_posicion(proxima_posicion(posicion_jugador, dir)) == BANDERA_AZUL) {
			ganador = ROJO;
			printf("GANADOR\n");
		}
		mover_jugador_tablero(posicion_jugador, proxima_posicion(posicion_jugador, dir), ROJO, nro_jugador);
	} else if (this->turno == AZUL && (en_posicion(proxima_posicion(posicion_jugador, dir)) == BANDERA_ROJA || es_color_libre(en_posicion(proxima_posicion(posicion_jugador, dir))))) { 
		// SI ES TURNO DEL AZUL Y EN LA COORDENADA A LA QUE SE QUIERE MOVER EL JUGADOR ES COLOR LIBRE O HAY BANDERA ROJO, ME MUEVO
		printf("MUEVO JUGADOR\n");
		if (en_posicion(proxima_posicion(posicion_jugador, dir)) == BANDERA_ROJA) {
			ganador = AZUL;
			printf("GANADOR\n");
		} 
		mover_jugador_tablero(posicion_jugador, proxima_posicion(posicion_jugador, dir), AZUL, nro_jugador);
	} else {
			// NO PUEDO MOVERME
			movimiento_valido = -1;
	}

	// Que no se puedan mover 2 jugadores a la vez
    // setear la variable ganador
    // Devolver acorde a la descripción
	return movimiento_valido;
}


void gameMaster::termino_ronda(color equipo) {
	// FIXME: Hacer chequeo de que es el color correcto que está llamando
	printf("color equipo: %i, color llamador: %i\n", this->turno, equipo);
	assert(equipo == this->turno);
	// FIXME: Hacer chequeo que hayan terminado todos los jugadores del equipo o su quantum (via mover_jugador)
	assert(this->turno == ROJO || this->turno == AZUL);
	
	if(this->turno == ROJO) {
		printf("jugador del equipo %i cambia de turno y hace wait en el semaforo\n+++++++++++++++++ CAMBIO TURNO +++++++++++++++++\n", equipo);
		this->turno = AZUL;
		this->turno_azul.unlock();
		this->turno_rojo.lock();
		printf("Vuelve a jugar el equipo %i\n", this->turno);
	} else if (this->turno == AZUL) {
		printf("jugador del equipo %i cambia de turno y hace wait en el semaforo\n+++++++++++++++++ CAMBIO TURNO +++++++++++++++++\n", equipo);
		this->turno = ROJO;
		this->turno_rojo.unlock();
		this->turno_azul.lock();
		printf("Vuelve a jugar el equipo %i\n", this->turno);
	}
}

bool gameMaster::termino_juego() {
	return ganador != INDEFINIDO;
}

coordenadas gameMaster::proxima_posicion(coordenadas anterior, direccion movimiento) {
	// Calcula la proxima posición a moverse (es una copia) 
	switch(movimiento) {
		case(ARRIBA):
			anterior.second++; 
			break;

		case(ABAJO):
			anterior.second--;
			break;

		case(IZQUIERDA):
			anterior.first--;
			break;

		case(DERECHA):
			anterior.first++;
			break;
	}
	return anterior; // está haciendo una copia por constructor
}

void gameMaster::update_quantum(int quantum_actual, color equipo_actual) {
	if (equipo_actual == ROJO) {
		this->quantum_rojo = quantum_actual;
	} else {
		this->quantum_azul = quantum_actual;
	}
}
