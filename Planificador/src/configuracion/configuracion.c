#include "configuracion.h"
#include "../esi/esi.h"

void bloquearClavesIniciales() {
	int i = 0;

	log_info(console_log, "Cargando claves inicialmente bloqueadas");

	while (planificador_setup.CLAVES_INICIALMENTE_BLOQUEADAS[i] != NULL) {
		bloquearRecurso(planificador_setup.CLAVES_INICIALMENTE_BLOQUEADAS[i]);
		i++;
	}
}

int leerArchivoConfiguracion(char* archivoConfiguracion) {

	if (archivoConfiguracion == NULL) {
		return -1;
	}

	log_info(console_log, "Cargando archivo de configuracion");

	t_config *configFile = config_create(archivoConfiguracion);

	if (configFile == NULL) {
		log_error(console_log, "No se pudo crear el archivo de configuracion");
		return -1;
	}

	log_info(console_log, " .:: Cargando settings ::.");

	planificador_setup.NOMBRE_INSTANCIA = malloc(30);
	planificador_setup.IP_COORDINADOR = malloc(30);

	// SI NO LO COPIO, CUANDO DESTRUYO EL CONFIG SE CORROMPEN
	strcpy(planificador_setup.NOMBRE_INSTANCIA,
			config_get_string_value(configFile, "NOMBRE_INSTANCIA"));
	strcpy(planificador_setup.IP_COORDINADOR,
			config_get_string_value(configFile, "IP_COORDINADOR"));
	planificador_setup.PUERTO_COORDINADOR = config_get_int_value(configFile,
			"PUERTO_COORDINADOR");
	planificador_setup.ESTIMACION_INICIAL = config_get_int_value(configFile,
			"ESTIMACION_INICIAL");
	planificador_setup.ALGORITMO_PLANIFICACION = config_get_int_value(
			configFile, "ALGORITMO_PLANIFICACION");
	planificador_setup.PUERTO_ESCUCHA_CONEXIONES = config_get_int_value(
			configFile, "PUERTO_ESCUCHA_CONEXIONES");
	planificador_setup.CLAVES_INICIALMENTE_BLOQUEADAS = config_get_array_value(
			configFile, "CLAVES_INICIALMENTE_BLOQUEADAS");
	planificador_setup.CANTIDAD_MAXIMA_CLIENTES = config_get_int_value(
			configFile, "CANTIDAD_MAXIMA_CLIENTES");
	planificador_setup.TAMANIO_COLA_CONEXIONES = config_get_int_value(
			configFile, "TAMANIO_COLA_CONEXIONES");
	planificador_setup.ALPHA = config_get_int_value(configFile, "ALPHA");
	planificador_setup.PUERTO_COORDINADOR_CONSOLA = config_get_int_value(
				configFile, "PUERTO_COORDINADOR_CONSOLA");

	config_destroy(configFile);

	return 0;
}

int cargarConfiguracion(char* archivoConfiguracion) {

	if (leerArchivoConfiguracion(archivoConfiguracion) > 0)
		return -1;

	mostrarConfiguracionPorConsola();

	bloquearClavesIniciales();

	return 0;
}

void mostrarConfiguracionPorConsola() {

	int i = 0;

	log_info(console_log, "\nSe cargó el setup del PLANIFICADOR");

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

	log_info(console_log, "\tAlpha: %d", planificador_setup.ALPHA);
}

void liberarRecursosConfiguracion() {
	free(planificador_setup.IP_COORDINADOR);
	free(planificador_setup.NOMBRE_INSTANCIA);
}
