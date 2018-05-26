#include "Instancia.h"

void print_header() {
	printf("\n\t\e[31;1m=========================================\e[0m\n");
	printf("\t.:: Bienvenido a ReDistinto ::.");
	printf("\t.:: Instancia -  ::.");
	printf("\n\t\e[31;1m=========================================\e[0m\n\n");
}

void print_goodbye() {
	printf("\n\t\e[31;1m=========================================\e[0m\n");
	printf("\t.:: Gracias por utilizar ReDistinto ::.");
	printf("\n\t\e[31;1m=========================================\e[0m\n\n");
}

void exit_program(int entero) {

	if (console_log != NULL)
		log_destroy(console_log);
	if (coordinator_socket != 0)
		close(coordinator_socket);

	dictionary_destroy(tabla_entradas);

	liberar_memoria();

	printf("\n\t\e[31;1m FINALIZA INSTANCIA \e[0m\n");
	exit(entero);
}

void create_log() {

	console_log = log_create("instancia.log", "ReDistinto-Instancia", true,
			LOG_LEVEL_TRACE);

	if (console_log == NULL) {
		printf(" FALLO - Creacion de Log");
		exit_program(EXIT_FAILURE);
	}
}

void loadConfig() {

	log_info(console_log, " Cargan datos del archivo de configuracion");

	t_config *config = config_create(PATH_FILE_NAME);

	if (config == NULL) {
		log_error(console_log,
				"FALLO - No se encontro la configuracion del log");
		exit_program(EXIT_FAILURE);
	}

	if (config != NULL) {
		instancia_setup.IP_COORDINADOR = string_duplicate(config_get_string_value(config,"IP_COORDINADOR"));
		instancia_setup.PUERTO_COORDINADOR = config_get_int_value(config,"PUERTO_COORDINADOR");
		instancia_setup.ALGORITMO_REEMPLAZO = config_get_int_value(config,"ALGORITMO_REEMPLAZO");
		instancia_setup.PUNTO_MONTAJE = config_get_string_value(config,"PUNTO_MONTAJE");
		instancia_setup.NOMBRE_INSTANCIA = string_duplicate(config_get_string_value(config,"NOMBRE_INSTANCIA"));
		instancia_setup.INTERVALO_DUMP_SEGs = config_get_int_value(config, "INTERVALO_DUMP_SEGs");

		log_info(console_log, " Carga exitosa de archivo de configuracion");
	}
	config_destroy(config);
}

void liberar_memoria() {
	// ADD

}

void log_inicial_consola() {

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

void init_structs(){

	tabla_entradas = dictionary_create();

}

void load_dump_files (){

	// TODO - 1. Se debe verificar que exista algun archivo y cargarlo a la tabla de entradas

}

void connect_with_coordinator() {


	log_info(console_log, "Connecting to Coordinador.");
	coordinator_socket = connect_to_server(instancia_setup.IP_COORDINADOR, instancia_setup.PUERTO_COORDINADOR, console_log);
	if(coordinator_socket <= 0){
		exit_program(EXIT_FAILURE);
	}

	if(!perform_connection_handshake(coordinator_socket, instancia_setup.NOMBRE_INSTANCIA, ESI, console_log)){
		exit_program(EXIT_FAILURE);
	}
	log_info(console_log, "Successfully connected to Coordinador.");

}

void send_example(){

	log_info(console_log, "Prepareo envio de prueba");

	t_response_process  abstract_response_intancia;
	abstract_response_intancia.instance_type = REDIS_INSTANCE;
	strcpy(abstract_response_intancia.response, "HOLA SOY INSTANCIA");

	void *buffer = serialize_abstract_request(&abstract_response_intancia);

	int result = send(coordinator_socket, buffer, CONNECTION_PACKAGE_SIZE, 0);
	free(buffer);

	if (result < CONNECTION_PACKAGE_SIZE) {
		log_error(console_log, "Could not send status response to Planner.");
	}

	log_info(console_log, "SE LE ENVIO EL MENSAJE OK AL COORDINADOR");

}

// INICIO DE PROCESO
int main(void) {

	print_header();
	create_log();
	loadConfig();
	log_inicial_consola();

	init_structs();
	load_dump_files();

	connect_with_coordinator();

	send_example();
	// 1. AL conectarse definir tamanio de entradas

	//Recibe sentencia
	//Extrae la clave
	//Identifica donde guardarlo. Si no hay espacio, le avisa al coordinador que tiene que compactar. Compacta
	//Guardar la clave
	//Envía el resultado al Coordinador

	print_goodbye();
	exit_program(EXIT_SUCCESS);

	return 0;
}

