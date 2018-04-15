#include "Planificador.h"

int readConfig(char* configFile) {
	if (configFile == NULL) {
		return -1;
	}
	t_config *config = config_create(configFile);
	log_info(console_log, " .:: Cargando settings ::.");

	if (config != NULL) {
		esi_setup.IP_COORDINADOR = config_get_string_value(config,
				"IP_COORDINADOR");
		esi_setup.PUERTO_COORDINADOR = config_get_int_value(config,
				"PUERTO_COORDINADOR");
		esi_setup.ESTIMACION_INICIAL = config_get_int_value(config,
				"ESTIMACION_INICIAL");
		esi_setup.ALGORITMO_PLANIFICACION = config_get_int_value(config,
				"ALGORITMO_PLANIFICACION");
		esi_setup.PUERTO_ESCUCHA_CONEXIONES = config_get_int_value(config,
						"PUERTO_ESCUCHA_CONEXIONES");
	}
	return 0;
}

int main(void) {
	console_log = log_create("esi.log", "ReDistinto-ESI", true,LOG_LEVEL_TRACE);
	printf("\n\t\e[31;1m=========================================\e[0m\n");
	printf("\t.:: Bievenido a ReDistinto ::.");
	printf("\n\t\e[31;1m=========================================\e[0m\n\n");
	if (readConfig(PATH_FILE_NAME) < 0) {
		log_error(console_log, "No se encontró el archivo de configuración");
		return -1;
	}
	log_info(console_log, "Se cargó el setup del PLANIFICADOR");

	log_info(console_log, "");

	log_info(console_log,
				"COORDINADOR: IP: %s, PUERTO: %d",
				esi_setup.IP_COORDINADOR, esi_setup.PUERTO_COORDINADOR);

	log_info(console_log, "Se cargó el setup con: ");

	switch(esi_setup.ALGORITMO_PLANIFICACION){
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

	log_info(console_log, "\tEstimacion inicial: %d", esi_setup.ESTIMACION_INICIAL);
	log_info(console_log, "\tPuerto conecciones: %d", esi_setup.PUERTO_ESCUCHA_CONEXIONES);

	log_info(console_log, "Se cerro la conexion con el planificador");
	printf("\n\t\e[31;1m Consola terminada. \e[0m\n");
	log_destroy(console_log);
	return 0;
}
