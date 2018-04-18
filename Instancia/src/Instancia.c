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


void liberar_memoria(){
	// TODO
}

void exit_program(int entero){

	printf("\n\t\e[31;1m Consola terminada. \e[0m\n");
	log_destroy(console_log);
	liberar_memoria();
	exit(entero);
}


void loguearConsolaInicial(){

	console_log = log_create("instancia.log", "ReDistinto-Instancia",true, LOG_LEVEL_TRACE);

	printf("\n\t\e[31;1m=========================================\e[0m\n");
	printf("\t.:: Bievenido a ReDistinto ::.");
	printf("\n\t\e[31;1m=========================================\e[0m\n\n");

	if (readConfig(PATH_FILE_NAME) < 0) {
		log_error(console_log, "No se encontró el archivo de configuración");
		exit_program(-1);
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


}



void conexion_coordinador(){

	int socket ;
	char *clientMessage = NULL;

	if(getClientSocket( &socket , instancia_setup.IP_COORDINADOR , instancia_setup.PUERTO_COORDINADOR)){
		exit_program(-1);
	}else{
		log_info(console_log , "COMUNICACION OK DE SERVER");
	}

	while(1) {
		printf("Ingrese mensaje : ");
	    fgets(clientMessage , 255 , stdin);
	    clientMessage[strlen(clientMessage) - 1] = '\0';

	    //Send some data
		if( send(socket , clientMessage , strlen(clientMessage) , 0) < 0) {
			log_error(console_log ,"ERROR EN LA COMUNICACION");
			exit_program(-1);
		}
	}

	exit_program(-1);
	free(clientMessage);
	close(socket);

}


// INICIO DE PROCESO
int main(void) {

	loguearConsolaInicial();

	conexion_coordinador();

	exit_program(1);

	return 0;
}


