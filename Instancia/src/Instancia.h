#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <signal.h>
#include <commons/config.h>
#include <commons/log.h>
#include "libs/socketCommons.h"
#include "libs/serialize.h"

#ifndef SRC_INSTANCIA_H_
#define SRC_INSTANCIA_H_

t_log *console_log;

/*MACROS*/
#define PATH_FILE_NAME "instancia.config"

/*ESTRUCTURAS*/
enum AlgortimoReemplazo {
	CIRC = 1, LRU = 2, BSU = 3
};

struct {
	char* IP_COORDINADOR;
	int PUERTO_COORDINADOR;
	enum AlgortimoReemplazo ALGORITMO_REEMPLAZO;
	char* PUNTO_MONTAJE;
	char* NOMBRE_INSTANCIA;
	int INTERVALO_DUMP_SEGs;
} instancia_setup;

void exit_program(int);

/*FUNCIONES*/
int readConfig(char* configFile);
void inicializacion();
void lectura_archivo();
void liberar_memoria();


/*UTILS*/
void loguearConsolaInicial();


/* CONEXION  */
void conexion_coordinador();




#endif /* SRC_INSTANCIA_H_ */
