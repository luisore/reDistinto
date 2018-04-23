/*
 * configuracion.h
 *
 *  Created on: 23 abr. 2018
 *      Author: leobriozzo
 */

#ifndef CONFIGURACION_CONFIGURACION_H_
#define CONFIGURACION_CONFIGURACION_H_

#include <commons/log.h>
#include <commons/config.h>

enum AlgortimoPlanificacion {
	SJF_CD = 1, SJF_SD = 2, HRRN = 3
};

struct CONFIGURACION {
	char* NOMBRE_INSTANCIA;
	char* IP_COORDINADOR;
	int PUERTO_COORDINADOR;
	enum AlgortimoPlanificacion ALGORITMO_PLANIFICACION;
	int ESTIMACION_INICIAL;
	int PUERTO_ESCUCHA_CONEXIONES;
	char** CLAVES_INICIALMENTE_BLOQUEADAS;
	int CANTIDAD_MAXIMA_CLIENTES;
	char** TAMANIO_COLA_CONEXIONES;
};

struct CONFIGURACION planificador_setup;

int cargarConfiguracion(t_log *console_log, char* archivoConfiguracion);

#endif /* CONFIGURACION_CONFIGURACION_H_ */