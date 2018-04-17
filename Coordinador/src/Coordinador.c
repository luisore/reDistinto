#include "Coordinador.h"


void print_header() {
	printf("\n\t\e[31;1m=========================================\e[0m\n");
	printf("\t.:: Bienvenido a ReDistinto ::.");
	printf("\t.:: Coordinador ::.");
	printf("\n\t\e[31;1m=========================================\e[0m\n\n");
}

void print_goodbye() {
	printf("\n\t\e[31;1m=========================================\e[0m\n");
	printf("\t.:: Gracias por utilizar ReDistinto ::.");
	printf("\n\t\e[31;1m=========================================\e[0m\n\n");
}

void exit_program(int entero) {

	if (coordinador_log != NULL)
		log_destroy(coordinador_log);

	liberar_memoria();

	printf("\n\t\e[31;1m FINALIZA COORDINAOR \e[0m\n");
	exit(entero);
}

void create_log() {

	coordinador_log = log_create("coodrinador.log", "ReDistinto-Coordinador", true,
			LOG_LEVEL_TRACE);

	if (coordinador_log == NULL) {
		printf(" FALLO - Creacion de Log");
		exit_program(EXIT_FAILURE);
	}
}

void loadConfig() {

	log_info(coordinador_log, " Cargan datos del archivo de configuracion");

	t_config *config = config_create(PATH_FILE_NAME);

	if (config == NULL) {
		log_error(coordinador_log,
				"FALLO - No se encontro la configuracion del log");
		exit_program(EXIT_FAILURE);
	}

	if (config != NULL) {
		coordinador_setup.NOMBRE_INSTANCIA = config_get_int_value(config,"NOMBRE_INSTANCIA");
		coordinador_setup.PUERTO_ESCUCHA_CONEXIONES = config_get_int_value(	config, "PUERTO_ESCUCHA_CONEXIONES");
		coordinador_setup.CANTIDAD_MAXIMA_CLIENTES = config_get_int_value(config,"CANTIDAD_MAXIMA_CLIENTES");
		coordinador_setup.TAMANIO_COLA_CONEXIONES = config_get_int_value(config,"TAMANIO_COLA_CONEXIONES");
		coordinador_setup.ALGORITMO_DISTRIBUCION = config_get_int_value(config,	"ALGORITMO_DISTRIBUCION");
		coordinador_setup.CANTIDAD_ENTRADAS = config_get_int_value(config,"CANTIDAD_ENTRADAS");
		coordinador_setup.TAMANIO_ENTRADA_BYTES = config_get_int_value(config,"TAMANIO_ENTRADA_BYTES");
		coordinador_setup.RETARDO_MS = config_get_int_value(config,	"RETARDO_MS");
	}
	config_destroy(config);
}

void liberar_memoria() {
	// ADD

}

void log_inicial_consola() {


	log_info(coordinador_log, "Se muestran los datos del coordinador");

	switch (coordinador_setup.ALGORITMO_DISTRIBUCION) {
	case LSU:
		log_info(coordinador_log, "\tAlgoritmo de distribucion: LSU");
		break;
	case EL:
		log_info(coordinador_log, "\tAlgoritmo de distribucion: EL");
		break;
	case KE:
		log_info(coordinador_log, "\tAlgoritmo de distribucion: KE");
		break;
	}

	log_info(coordinador_log, "\tCantidad de entradas: %d",	coordinador_setup.CANTIDAD_ENTRADAS);
	log_info(coordinador_log, "\tTamanio de entrada en bytes: %d", coordinador_setup.TAMANIO_ENTRADA_BYTES);
	log_info(coordinador_log, "\tRetardo en milis: %d", coordinador_setup.RETARDO_MS);

}


void create_tcp_server(){

	server = tcpserver_create(coordinador_setup.NOMBRE_INSTANCIA, coordinador_log,
			coordinador_setup.CANTIDAD_MAXIMA_CLIENTES,
			coordinador_setup.TAMANIO_COLA_CONEXIONES,
			coordinador_setup.PUERTO_ESCUCHA_CONEXIONES, true);

	if(server == NULL){
		log_error(coordinador_log, "Could not create TCP server. Aborting execution.");
		exit_program(EXIT_FAILURE);
	}
}




int main(void) {

	print_header();
	create_log();
	loadConfig();
	log_inicial_consola();


	create_tcp_server();
	//tcpserver_run(server, before_tpc_server_cycle, on_server_accept, on_server_read, on_server_command);



	print_goodbye();
	exit_program(EXIT_SUCCESS);

	return 0;

}
