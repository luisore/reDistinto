#include "commons.h"

int create_log() {
	console_log = log_create("planificador.log", "ReDistinto-Planificador",
	false, LOG_LEVEL_TRACE);

	if (console_log == NULL) {
		printf("No se pudo crear el log. Abortando ejecución\n");
		return EXIT_FAILURE;
	}

	info_log("Log creado correctamente");

	return 0;
}

void info_log(char * message){
	pthread_mutex_lock(&mutexLog);
	log_info(console_log, message);
	pthread_mutex_unlock(&mutexLog);
}

void info_log_param1(char * message, void * param1){
	pthread_mutex_lock(&mutexLog);
	log_info(console_log, message, param1);
	pthread_mutex_unlock(&mutexLog);
}

void info_log_param2(char * message, void * param1, void * param2){
	pthread_mutex_lock(&mutexLog);
	log_info(console_log, message, param1, param2);
	pthread_mutex_unlock(&mutexLog);
}

void error_log(char * message){
	pthread_mutex_lock(&mutexLog);
	log_error(console_log, message);
	pthread_mutex_unlock(&mutexLog);
}

void error_log_param1(char * message, void * param1){
	pthread_mutex_lock(&mutexLog);
	log_error(console_log, message, param1);
	pthread_mutex_unlock(&mutexLog);
}

void error_log_param2(char * message, void * param1, void * param2){
	pthread_mutex_lock(&mutexLog);
	log_error(console_log, message, param1, param2);
	pthread_mutex_unlock(&mutexLog);
}

char * operacionAString(operation_type_e tipo_operacion)
{
	switch(tipo_operacion){
	case GET: return "GET";
	case SET: return "SET";
	case STORE: return "STORE";
	default: return "";
	}
}

