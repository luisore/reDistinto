#include "Planificador.h"

int readConfig(char* configFile) {
	if (configFile == NULL) {
		return -1;
	}
	t_config *config = config_create(configFile);
	log_info(console_log, " .:: Cargando settings ::.");

	if (config != NULL) {
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
	}
	return 0;
}

int main(void) {
	console_log = log_create("planificador.log", "ReDistinto-Planificador",
	true, LOG_LEVEL_TRACE);
	printf("\n\t\e[31;1m=========================================\e[0m\n");
	printf("\t.:: Bievenido a ReDistinto ::.");
	printf("\n\t\e[31;1m=========================================\e[0m\n\n");
	if (readConfig(PATH_FILE_NAME) < 0) {
		log_error(console_log, "No se encontró el archivo de configuración");
		return -1;
	}
	log_info(console_log, "Se cargó el setup del PLANIFICADOR");

	log_info(console_log, "");

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

	int i = 0;
	while (planificador_setup.CLAVES_INICIALMENTE_BLOQUEADAS[i] != NULL) {
		log_info(console_log, "\tClave inicial bloqueada nro %d: %s", i + 1,
				planificador_setup.CLAVES_INICIALMENTE_BLOQUEADAS[i]);
		i++;
	}

	log_info(console_log, "Se cerro la conexion con el planificador");
	printf("\n\t\e[31;1m Consola terminada. \e[0m\n");

	log_destroy(console_log);
	return 0;
}
