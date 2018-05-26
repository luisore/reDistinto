#include "commons.h"

int create_log() {
	console_log = log_create("planificador.log", "ReDistinto-Planificador",
	true, LOG_LEVEL_TRACE);

	if (console_log == NULL) {
		printf("Could not create log. Execution aborted.");
		return EXIT_FAILURE;
	}

	return 0;
}
