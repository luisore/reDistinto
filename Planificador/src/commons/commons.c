#include "commons.h"

int create_log() {
	console_log = log_create("planificador.log", "ReDistinto-Planificador",
	false, LOG_LEVEL_TRACE);

	if (console_log == NULL) {
		log_info(console_log, "No se pudo crear el log. Abortando ejecución");
		return EXIT_FAILURE;
	}

	return 0;
}


