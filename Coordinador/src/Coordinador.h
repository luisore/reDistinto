#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <signal.h>
#include <commons/config.h>
#include <commons/log.h>
#include "libs/socketCommons.h"
#include "libs/serialize.h"

#ifndef SRC_COORDINADOR_H_
#define SRC_COORDINADOR_H_

/*MACROS*/
#define PATH_FILE_NAME "coordinador.config"

/*VARIABLES GLOBALES*/
t_log *console_log;

/*ESTRUCTURAS*/

enum AlgortimoDistribucion {
	LSU = 1, EL = 2, KE = 3
};

struct {
	int PUERTO_ESCUCHA_CONEXIONES;
	enum AlgortimoDistribucion ALGORITMO_DISTRIBUCION;
	int CANTIDAD_ENTRADAS;
	int TAMANIO_ENTRADA_BYTES;
	int RETARDO_MS;
} coordinador_setup;

/*FUNCIONES*/
int readConfig(char* configFile);

#endif /* SRC_COORDINADOR_H_ */