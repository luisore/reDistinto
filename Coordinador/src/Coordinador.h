#include <stdio.h> //printf
#include <string.h>    //strlen
#include <stdbool.h>
#include <signal.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include "libs/serialize.h"
#include "libs/tcpserver.h"
#include <sys/socket.h> // Para crear sockets, enviar, recibir, etc
#include <netdb.h> // Para getaddrinfo
#include <unistd.h> // Para close
#include "libs/protocols.h"

#ifndef SRC_COORDINADOR_H_
#define SRC_COORDINADOR_H_

/*MACROS*/
#define PATH_FILE_NAME "coordinador.config"

/*VARIABLES GLOBALES*/
t_log * coordinador_log;
t_config *config;

tcp_server_t* server;


/*ESTRUCTURAS*/



enum AlgortimoDistribucion {
	LSU = 1, EL = 2, KE = 3
};

struct {
	char * NOMBRE_INSTANCIA;
	int PUERTO_ESCUCHA_CONEXIONES;
	int CANTIDAD_MAXIMA_CLIENTES;
	int TAMANIO_COLA_CONEXIONES;
	enum AlgortimoDistribucion ALGORITMO_DISTRIBUCION;
	int CANTIDAD_ENTRADAS;
	int TAMANIO_ENTRADA_BYTES;
	int RETARDO_MS;
} coordinador_setup;


/*FUNCIONES GENERALES*/

void print_header();
void create_log();
void loadConfig();
void log_inicial_consola();

void print_goodbye();
void exit_program(int);

void liberar_memoria();





#endif /* SRC_COORDINADOR_H_ */
