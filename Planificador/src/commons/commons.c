#include "commons.h"

int create_log() {
	console_log = log_create("planificador.log", "ReDistinto-Planificador",
	false, LOG_LEVEL_TRACE);

	if (console_log == NULL) {
		printf("No se pudo crear el log. Abortando ejecución\n");
		return EXIT_FAILURE;
	}

	log_info(console_log, "Log creado correctamente");

	return 0;
}


