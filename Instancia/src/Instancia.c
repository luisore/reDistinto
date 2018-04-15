#include "Instancia.h"

int readConfig(char* configFile) {
	if (configFile == NULL) {
		return -1;
	}
	t_config *config = config_create(configFile);
	log_info(console_log, " .:: Cargando settings ::.");

	if (config != NULL) {
		instancia_setup.IP_COORDINADOR = config_get_string_value(config,
				"IP_COORDINADOR");
		instancia_setup.PUERTO_COORDINADOR = config_get_int_value(config,
				"PUERTO_COORDINADOR");
		instancia_setup.ALGORITMO_REEMPLAZO = config_get_int_value(config,
				"ALGORITMO_REEMPLAZO");
		instancia_setup.PUNTO_MONTAJE = config_get_string_value(config,
				"PUNTO_MONTAJE");
		instancia_setup.NOMBRE_INSTANCIA = config_get_string_value(config,
				"NOMBRE_INSTANCIA");
		instancia_setup.INTERVALO_DUMP_SEGs = config_get_int_value(config,
				"INTERVALO_DUMP_SEGs");
	}
	return 0;
}

int main(void) {
	console_log = log_create("instancia.log", "ReDistinto-Instancia",
	true, LOG_LEVEL_TRACE);
	printf("\n\t\e[31;1m=========================================\e[0m\n");
	printf("\t.:: Bievenido a ReDistinto ::.");
	printf("\n\t\e[31;1m=========================================\e[0m\n\n");
	if (readConfig(PATH_FILE_NAME) < 0) {
		log_error(console_log, "No se encontró el archivo de configuración");
		return -1;
	}
	log_info(console_log, "Se cargó el setup del INSTANCIA");

	log_info(console_log, "");

	log_info(console_log, "\tCOORDINADOR: IP: %s, PUERTO: %d",
			instancia_setup.IP_COORDINADOR, instancia_setup.PUERTO_COORDINADOR);

	switch (instancia_setup.ALGORITMO_REEMPLAZO) {
	case CIRC:
		log_info(console_log, "\tAlgoritmo de reemplazo: CIRC");
		break;
	case LRU:
		log_info(console_log, "\tAlgoritmo de reemplazo: LRU");
		break;
	case BSU:
		log_info(console_log, "\tAlgoritmo de planificacion: BSU");
		break;
	}

	log_info(console_log, "\tPunto de montaje: %s",
			instancia_setup.PUNTO_MONTAJE);
	log_info(console_log, "\tNombre de la instancia: %s",
			instancia_setup.NOMBRE_INSTANCIA);
	log_info(console_log, "\tIntervalo de dump en segundos: %d",
				instancia_setup.INTERVALO_DUMP_SEGs);

	log_info(console_log, "Se cerro la conexion con el Coordinador");
	printf("\n\t\e[31;1m Consola terminada. \e[0m\n");

	log_destroy(console_log);
	return 0;
}
