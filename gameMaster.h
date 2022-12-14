#ifndef GAMEMASTER_H
#define GAMEMASTER_H
#include <tuple>
#include <cstdio>
#include <vector>
#include <mutex>
#include <semaphore>
#include "definiciones.h"
#include "config.h"
#include <thread>
#include <semaphore>

using namespace std;

class gameMaster {
private:
    // Atributos Privados
    int nro_ronda = 0;
    int x, y, jugadores_por_equipos;
    vector<vector<color>> tablero;
    vector<coordenadas> pos_jugadores_azules, pos_jugadores_rojos;
    coordenadas pos_bandera_roja, pos_bandera_azul;
    color turno;
    estrategia strat;
	
    //
    //...
    //

    int quantum_rojo, quantum_azul;

    // Métodos privados
    color obtener_coordenadas(coordenadas coord);
    void mover_jugador_tablero(coordenadas pos_anterior, coordenadas pos_nueva, color colorEquipo, int nro_jugador);
    //
    //...
    //
 
public:
    // Atributos públicos
    gameMaster(Config config);
    color ganador = INDEFINIDO;    

    //
    //...
    //

    // mutex turno_rojo, turno_azul;
    std::binary_semaphore turno_rojo{1}, turno_azul{1};
    mutex mov_jugador;

    // Métodos públicos
    void termino_ronda(color equipo); // Marca que un jugador terminó la ronda
    int mover_jugador(direccion dir, int nro_jugador);
    bool termino_juego();
	int getTamx();
	int getTamy();
    static int distancia(coordenadas pair1, coordenadas pair2);
    color en_posicion(coordenadas coord);
    bool es_posicion_valida(coordenadas pos);
    bool es_color_libre(color color_tablero);
	coordenadas proxima_posicion(coordenadas anterior, direccion movimiento); // Calcula la proxima posición a moverse	
    //
    //...
    //

    void update_quantum(int quantum_actual, color equipo_actual);
};

#endif // GAMEMASTER_H
