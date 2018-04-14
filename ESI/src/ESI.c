#include "ESI.h"

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
		esi_setup.IP_PLANIFICADOR = config_get_string_value(config,
				"IP_PLANIFICADOR");
		esi_setup.PUERTO_PLANIFICADOR = config_get_int_value(config,
				"PUERTO_PLANIFICADOR");
	}
	return 0;
}

int main(void) {
	console_log = log_create("esi.log", "ReDistinto-ESI", true,
			LOG_LEVEL_TRACE);
	printf("\n\t\e[31;1m=========================================\e[0m\n");
	printf("\t.:: Bievenido a ReDistinto ::.");
	printf("\n\t\e[31;1m=========================================\e[0m\n\n");
	if (readConfig(PATH_FILE_NAME) < 0) {
		log_error(console_log, "No se encontró el archivo de configuración");
		return -1;
	}
	log_info(console_log,
			"Se cargó el setup con IP %s y puerto %d del COORDINADOR",
			esi_setup.IP_COORDINADOR, esi_setup.PUERTO_COORDINADOR);

	log_info(console_log, "Se cerro la conexion con el coordinador");

	log_info(console_log,
			"Se cargó el setup con IP %s y puerto %d del PLANIFICADOR",
			esi_setup.IP_PLANIFICADOR, esi_setup.PUERTO_PLANIFICADOR);

	log_info(console_log, "Se cerro la conexion con el planificador");
	printf("\n\t\e[31;1m Consola terminada. \e[0m\n");
	log_destroy(console_log);
	return 0;
}
