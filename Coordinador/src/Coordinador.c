#include "Coordinador.h"

int readConfig(char* configFile) {
	if (configFile == NULL) {
		return -1;
	}
	t_config *config = config_create(configFile);
	log_info(console_log, " .:: Cargando settings ::.");

	if (config != NULL) {
		coordinador_setup.PUERTO_ESCUCHA_CONEXIONES = config_get_int_value(
				config, "PUERTO_ESCUCHA_CONEXIONES");
		coordinador_setup.ALGORITMO_DISTRIBUCION = config_get_int_value(config,
				"ALGORITMO_DISTRIBUCION");
		coordinador_setup.CANTIDAD_ENTRADAS = config_get_int_value(config,
				"CANTIDAD_ENTRADAS");
		coordinador_setup.TAMANIO_ENTRADA_BYTES = config_get_int_value(config,
				"TAMANIO_ENTRADA_BYTES");
		coordinador_setup.RETARDO_MS = config_get_int_value(config,
				"RETARDO_MS");
	}
	config_destroy(config);
	return 0;
}

int main(void) {
	console_log = log_create("coordinador.log", "ReDistinto-Coordinador",
	true, LOG_LEVEL_TRACE);
	printf("\n\t\e[31;1m=========================================\e[0m\n");
	printf("\t.:: Bievenido a ReDistinto ::.");
	printf("\n\t\e[31;1m=========================================\e[0m\n\n");
	if (readConfig(PATH_FILE_NAME) < 0) {
		log_error(console_log, "No se encontró el archivo de configuración");
		return -1;
	}
	log_info(console_log, "Se cargó el setup del COORDINADOR");

	log_info(console_log, "");

	log_info(console_log, "\tPuerto conecciones: %d",
			coordinador_setup.PUERTO_ESCUCHA_CONEXIONES);

	switch (coordinador_setup.ALGORITMO_DISTRIBUCION) {
	case LSU:
		log_info(console_log, "\tAlgoritmo de distribucion: LSU");
		break;
	case EL:
		log_info(console_log, "\tAlgoritmo de distribucion: EL");
		break;
	case KE:
		log_info(console_log, "\tAlgoritmo de distribucion: KE");
		break;
	}

	log_info(console_log, "\tCantidad de entradas: %d",
			coordinador_setup.CANTIDAD_ENTRADAS);

	log_info(console_log, "\tTamanio de entrada en bytes: %d",
			coordinador_setup.TAMANIO_ENTRADA_BYTES);

	log_info(console_log, "\tRetardo en milis: %d",
			coordinador_setup.RETARDO_MS);

	log_info(console_log, "Se cerro la conexion con el planificador");
	printf("\n\t\e[31;1m Consola terminada. \e[0m\n");

	log_destroy(console_log);
	return 0;
}
