#ifndef CONFIGURACION_CONFIGURACION_H_
#define CONFIGURACION_CONFIGURACION_H_

#include <commons/log.h>
#include <commons/config.h>
#include <string.h>

enum AlgortimoPlanificacion {
	SJF_SD = 0, SJF_CD = 1, HRRN = 2
};

typedef struct {
	char* NOMBRE_INSTANCIA;
	char* IP_COORDINADOR;
	int PUERTO_COORDINADOR;
	enum AlgortimoPlanificacion ALGORITMO_PLANIFICACION;
	int ESTIMACION_INICIAL;
	int PUERTO_ESCUCHA_CONEXIONES;
	char** CLAVES_INICIALMENTE_BLOQUEADAS;
	int CANTIDAD_MAXIMA_CLIENTES;
	int TAMANIO_COLA_CONEXIONES;
	int ALPHA;
} CONFIGURACION;

CONFIGURACION planificador_setup;

int cargarConfiguracion(char* archivoConfiguracion);
void mostrarConfiguracionPorConsola();
void liberarRecursosConfiguracion();

#endif /* CONFIGURACION_CONFIGURACION_H_ */
