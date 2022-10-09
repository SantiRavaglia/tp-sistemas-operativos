#include <sys/unistd.h>
#include <assert.h>     /* assert */
#include "gameMaster.h"

// SE FIJA SI LA COORDENADA ESTA DENTRO DEL TABLERO
bool gameMaster::es_posicion_valida(coordenadas pos) {
	return (pos.first > 0) && (pos.first < x) && (pos.second > 0) && (pos.second < y);
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
	this->turno = ROJO;

    cout << "SE HA INICIALIZADO GAMEMASTER CON EXITO" << endl;
    // Insertar código que crea necesario de inicialización 
}

// MUEVE JUGADOR 
void gameMaster::mover_jugador_tablero(coordenadas pos_anterior, coordenadas pos_nueva, color colorEquipo){
    assert(es_color_libre(tablero[pos_nueva.first][pos_nueva.second]));
    tablero[pos_anterior.first][pos_anterior.second] = VACIO; 
    tablero[pos_nueva.first][pos_nueva.second] = colorEquipo;
}


int gameMaster::mover_jugador(direccion dir, int nro_jugador) {
	this->m_turno.lock();
	// Chequear que la movida sea valida
	coordenadas posicion_jugador;
	if (turno == ROJO) {
		posicion_jugador = this->pos_jugadores_rojos[nro_jugador];
	} else 	{
		posicion_jugador = this->pos_jugadores_azules[nro_jugador];
	}
	switch (dir) {
		case ARRIBA: 
			if (this->turno == ROJO && this->en_posicion(make_pair(posicion_jugador.first+1, posicion_jugador.second)) == BANDERA_AZUL && es_color_libre(en_posicion(make_pair(posicion_jugador.first+1, posicion_jugador.second)))) {
				// SI ES TURNO DEL ROJO Y EN LA COORDENADA A LA QUE SE QUIERE MOVER EL JUGADOR ES COLOR LIBRE O HAY BANDERA AZUL, ME MUEVO
				mover_jugador_tablero(posicion_jugador, make_pair(posicion_jugador.first+1, posicion_jugador.second), ROJO);
				if (this->en_posicion(make_pair(posicion_jugador.first+1, posicion_jugador.second)) == BANDERA_AZUL) {
					this->ganador = ROJO;
				}
			} else if (this->turno == AZUL && this->en_posicion(make_pair(posicion_jugador.first+1, posicion_jugador.second)) == BANDERA_ROJA && es_color_libre(en_posicion(make_pair(posicion_jugador.first+1, posicion_jugador.second)))) { 
				// SI ES TURNO DEL AZUL Y EN LA COORDENADA A LA QUE SE QUIERE MOVER EL JUGADOR ES COLOR LIBRE O HAY BANDERA ROJO, ME MUEVO
				mover_jugador_tablero(posicion_jugador, make_pair(posicion_jugador.first+1, posicion_jugador.second), AZUL);
				if (this->en_posicion(make_pair(posicion_jugador.first+1, posicion_jugador.second)) == BANDERA_ROJA) {
					this->ganador = AZUL;
				} else {
					// NO PUEDO MOVERME
					return -1;
				}
			}

		break;

		case ABAJO: 
			if (this->turno == ROJO && this->en_posicion(make_pair(posicion_jugador.first-1, posicion_jugador.second)) == BANDERA_AZUL && es_color_libre(en_posicion(make_pair(posicion_jugador.first-1, posicion_jugador.second)))) {
				// SI ES TURNO DEL ROJO Y EN LA COORDENADA A LA QUE SE QUIERE MOVER EL JUGADOR ES COLOR LIBRE O HAY BANDERA AZUL, ME MUEVO
				mover_jugador_tablero(posicion_jugador, make_pair(posicion_jugador.first-1, posicion_jugador.second), ROJO);
				if (this->en_posicion(make_pair(posicion_jugador.first+1, posicion_jugador.second)) == BANDERA_AZUL) {
					this->ganador = ROJO;
				}
			} else if (this->turno == AZUL && this->en_posicion(make_pair(posicion_jugador.first-1, posicion_jugador.second)) == BANDERA_ROJA && es_color_libre(en_posicion(make_pair(posicion_jugador.first-1, posicion_jugador.second)))) { 
				// SI ES TURNO DEL AZUL Y EN LA COORDENADA A LA QUE SE QUIERE MOVER EL JUGADOR ES COLOR LIBRE O HAY BANDERA ROJO, ME MUEVO
				mover_jugador_tablero(posicion_jugador, make_pair(posicion_jugador.first-1, posicion_jugador.second), AZUL);
				if (this->en_posicion(make_pair(posicion_jugador.first+1, posicion_jugador.second)) == BANDERA_ROJA) {
					this->ganador = AZUL;
				} else { 
					// NO PUEDO MOVERME
					return -1;
				}
			}

		break;

		case IZQUIERDA: 
			if (this->turno == ROJO && this->en_posicion(make_pair(posicion_jugador.first, posicion_jugador.second-1)) == BANDERA_AZUL && es_color_libre(en_posicion(make_pair(posicion_jugador.first, posicion_jugador.second-1)))) {
				// SI ES TURNO DEL ROJO Y EN LA COORDENADA A LA QUE SE QUIERE MOVER EL JUGADOR ES COLOR LIBRE O HAY BANDERA AZUL, ME MUEVO
				mover_jugador_tablero(posicion_jugador, make_pair(posicion_jugador.first, posicion_jugador.second-1), ROJO);
				if (this->en_posicion(make_pair(posicion_jugador.first+1, posicion_jugador.second)) == BANDERA_AZUL) {
					this->ganador = ROJO;
				}
			} else if (this->turno == AZUL && this->en_posicion(make_pair(posicion_jugador.first, posicion_jugador.second-1)) == BANDERA_ROJA && es_color_libre(en_posicion(make_pair(posicion_jugador.first, posicion_jugador.second-1)))) { 
				// SI ES TURNO DEL AZUL Y EN LA COORDENADA A LA QUE SE QUIERE MOVER EL JUGADOR ES COLOR LIBRE O HAY BANDERA ROJO, ME MUEVO
				mover_jugador_tablero(posicion_jugador, make_pair(posicion_jugador.first, posicion_jugador.second-1), AZUL);
				if (this->en_posicion(make_pair(posicion_jugador.first+1, posicion_jugador.second)) == BANDERA_ROJA) {
					this->ganador = AZUL;
				} else {
					// NO PUEDO MOVERME
					return -1;
				}
			}

		break;

		case DERECHA: 
			if (this->turno == ROJO && this->en_posicion(make_pair(posicion_jugador.first, posicion_jugador.second+1)) == BANDERA_AZUL && es_color_libre(en_posicion(make_pair(posicion_jugador.first, posicion_jugador.second+1)))) {
				// SI ES TURNO DEL ROJO Y EN LA COORDENADA A LA QUE SE QUIERE MOVER EL JUGADOR ES COLOR LIBRE O HAY BANDERA AZUL, ME MUEVO
				mover_jugador_tablero(posicion_jugador, make_pair(posicion_jugador.first, posicion_jugador.second+1), ROJO);
				if (this->en_posicion(make_pair(posicion_jugador.first+1, posicion_jugador.second)) == BANDERA_AZUL) {
					this->ganador = ROJO;
				}
			} else if (this->turno == AZUL && this->en_posicion(make_pair(posicion_jugador.first, posicion_jugador.second+1)) == BANDERA_ROJA && es_color_libre(en_posicion(make_pair(posicion_jugador.first, posicion_jugador.second+1)))) { 
				// SI ES TURNO DEL AZUL Y EN LA COORDENADA A LA QUE SE QUIERE MOVER EL JUGADOR ES COLOR LIBRE O HAY BANDERA ROJO, ME MUEVO
				mover_jugador_tablero(posicion_jugador, make_pair(posicion_jugador.first, posicion_jugador.second+1), AZUL);
				if (this->en_posicion(make_pair(posicion_jugador.first+1, posicion_jugador.second)) == BANDERA_ROJA) {
					this->ganador = AZUL;
				} else {
					// NO PUEDO MOVERME
					return -1;
				}
			}

		break;
	}
	// Que no se puedan mover 2 jugadores a la vez
    // setear la variable ganador
    // Devolver acorde a la descripción
	this->m_turno.unlock();
	return 0;
}


void gameMaster::termino_ronda(color equipo) {
	// FIXME: Hacer chequeo de que es el color correcto que está llamando
	// FIXME: Hacer chequeo que hayan terminado todos los jugadores del equipo o su quantum (via mover_jugador)
}

bool gameMaster::termino_juego() {
	return ganador != INDEFINIDO;
}

coordenadas gameMaster::proxima_posicion(coordenadas anterior, direccion movimiento) {
	// Calcula la proxima posición a moverse (es una copia) 
	switch(movimiento) {
		case(ARRIBA):
			anterior.second--; 
			break;

		case(ABAJO):
			anterior.second++;
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

