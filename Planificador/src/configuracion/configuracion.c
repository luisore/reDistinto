/*
 * configuracion.c
 *
 *  Created on: 23 abr. 2018
 *      Author: leobriozzo
 */

#include "configuracion.h"

int leerArchivoConfiguracion(t_log *console_log, char* archivoConfiguracion) {

	t_config *config = NULL;

	if (archivoConfiguracion == NULL) {
		return -1;
	}

	config = config_create(archivoConfiguracion);

	log_info(console_log, " .:: Cargando settings ::.");

	if (config != NULL) {
		planificador_setup.NOMBRE_INSTANCIA = config_get_string_value(config,
				"NOMBRE_INSTANCIA");
		planificador_setup.IP_COORDINADOR = config_get_string_value(config,
				"IP_COORDINADOR");
		planificador_setup.PUERTO_COORDINADOR = config_get_int_value(config,
				"PUERTO_COORDINADOR");
		planificador_setup.ESTIMACION_INICIAL = config_get_int_value(config,
				"ESTIMACION_INICIAL");
		planificador_setup.ALGORITMO_PLANIFICACION = config_get_int_value(
				config, "ALGORITMO_PLANIFICACION");
		planificador_setup.PUERTO_ESCUCHA_CONEXIONES = config_get_int_value(
				config, "PUERTO_ESCUCHA_CONEXIONES");
		planificador_setup.CLAVES_INICIALMENTE_BLOQUEADAS =
				config_get_array_value(config,
						"CLAVES_INICIALMENTE_BLOQUEADAS");
		planificador_setup.CANTIDAD_MAXIMA_CLIENTES = config_get_int_value(
				config, "CANTIDAD_MAXIMA_CLIENTES");
		planificador_setup.TAMANIO_COLA_CONEXIONES = config_get_int_value(
				config, "TAMANIO_COLA_CONEXIONES");
	}

	config_destroy(config);

	return 0;
}

int cargarConfiguracion(t_log *console_log, char* archivoConfiguracion) {

	if (leerArchivoConfiguracion(console_log, archivoConfiguracion) > 0)
		return -1;

	mostrarConfiguracionPorConsola(console_log);

	return 0;
}

void mostrarConfiguracionPorConsola(t_log * console_log) {

	int i = 0;

	log_info(console_log, "Se cargó el setup del PLANIFICADOR");

	log_info(console_log, "\tNombre de instancia: %s",
			planificador_setup.NOMBRE_INSTANCIA);

	log_info(console_log, "\tCOORDINADOR: IP: %s, PUERTO: %d",
			planificador_setup.IP_COORDINADOR,
			planificador_setup.PUERTO_COORDINADOR);

	switch (planificador_setup.ALGORITMO_PLANIFICACION) {
	case SJF_CD:
		log_info(console_log, "\tAlgoritmo de planificacion: SJF_CD");
		break;
	case SJF_SD:
		log_info(console_log, "\tAlgoritmo de planificacion: SJF_SD");
		break;
	case HRRN:
		log_info(console_log, "\tAlgoritmo de planificacion: HRRN");
		break;
	}

	log_info(console_log, "\tEstimacion inicial: %d",
			planificador_setup.ESTIMACION_INICIAL);
	log_info(console_log, "\tPuerto conecciones: %d",
			planificador_setup.PUERTO_ESCUCHA_CONEXIONES);

	while (planificador_setup.CLAVES_INICIALMENTE_BLOQUEADAS[i] != NULL) {
		log_info(console_log, "\tClave inicial bloqueada nro %d: %s", i + 1,
				planificador_setup.CLAVES_INICIALMENTE_BLOQUEADAS[i]);
		i++;
	}

	log_info(console_log, "\tCantidad maxima de clientes: %d",
			planificador_setup.CANTIDAD_MAXIMA_CLIENTES);

	log_info(console_log, "\tTamanio de la cola de conexiones: %d",
			planificador_setup.TAMANIO_COLA_CONEXIONES);
}
