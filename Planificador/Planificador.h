#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <signal.h>
#include <commons/config.h>
#include <commons/log.h>
#include "libs/socketCommons.h"
#include "libs/serialize.h"

/*MACROS*/
#define PATH_FILE_NAME "planificador.config"

#ifndef PLANIFICADOR_SRC_PLANIFICADOR_H_
#define PLANIFICADOR_SRC_PLANIFICADOR_H_

t_log *console_log;

enum AlgortimoPlanificacion {
	SJF_CD = 1,
	SJF_SD = 2,
	HRRN = 3
} ;

struct {
	char* IP_COORDINADOR;
	int PUERTO_COORDINADOR;
	enum AlgortimoPlanificacion ALGORITMO_PLANIFICACION;
	int ESTIMACION_INICIAL;
	int PUERTO_ESCUCHA_CONEXIONES;

} esi_setup;

//CLAVES_INICIALMENTE_BLOQUEADAS=materias:K3002,materias:K3001

/*FUNCIONES*/
int readConfig(char* configFile);


#endif /* PLANIFICADOR_SRC_PLANIFICADOR_H_ */
