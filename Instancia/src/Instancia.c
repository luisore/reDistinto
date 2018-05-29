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
	dictionary_destroy(tabla_claves);

	// Destroy every element
	for(int i=0 ; i < list_size(lista_entradas) ; i++){
		t_entrada * entrada = list_get(lista_entradas, i);
		free(entrada->valor);
		free(entrada);
	}

	list_destroy(lista_entradas);

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

void build_tabla_entradas(){

	log_info(console_log, "Se arma la estructura de entradas");
	log_info(console_log, "\t Cantidad total: %d" , storage);
	log_info(console_log, "\t Tamanio entradas: %d" , tamanio_entradas);
	log_info(console_log, "\t Cantidad de entradas: %d" , cantidad_entradas);

	for(int i=0 ; i < cantidad_entradas ; i++){

		log_info(console_log, "Se carga entrada:  %d" , i);

		t_entrada* entrada = malloc(sizeof(t_entrada));
		entrada->siguiente_instruccion = 0;
		entrada->tamanio = tamanio_entradas;
		entrada->valor = malloc(tamanio_entradas);

		//strcpy(entrada->valor , "PRUEBA VALOR");

		// SE CARGAN LAS ESTRUCTURAS

		list_add(lista_entradas, entrada);

		char clave[cantidad_entradas];
		sprintf(clave, "%d", i);

		// @param i  ->  identificador entrada
		// @param 0  ->  valor de uso de la entrada ( 0 o 1 ) por si o por no
		dictionary_put(tabla_entradas ,clave , 0);

		// nota -> Deberia poder obtener el valor directamente de la posicion del dictionario

	}

	log_info(console_log, "TERMINA LA CARGA DE ESTRUCTURAS");


}

void show_structs(){

	int a = 0;

	for (a= 0 ; a < list_size(lista_entradas) ; a++){

		t_entrada * entrada = list_get(lista_entradas , a);

		log_info(console_log, "ENTRADA NUMERO: %d" , a);
		log_info(console_log, "TAMANIO: %d" ,entrada->tamanio);
		log_info(console_log, "VALOR: %s", entrada->valor);

	}

}

void init_structs(){

	tabla_entradas = dictionary_create();
	tabla_claves = dictionary_create();
	lista_entradas = list_create();

	// EJEMPLO CARGA

	storage = 500;
	tamanio_entradas = 50;
	cantidad_entradas = rint(storage / tamanio_entradas);

	build_tabla_entradas();

	// EJEMPLO DE VISUALIZACION

	show_structs();
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

bool existe_capacidad_valor(char * valor){
	// TODO
	return true;
}

void reemplazar_por_algoritmo(){

	switch(instancia_setup.ALGORITMO_REEMPLAZO){
		case CIRC:
			log_info(console_log, "Comienza reemplazo con algoritmo CIRCULAR");
			reemplazoCircular();
			break;
		case LRU:
			log_info(console_log, "Comienza reemplazo con algoritmo LAST RECENTLY USED");
			reemplazoLeastRecentlyUsed();
			break;
		case BSU:
			log_info(console_log, "Comienza reemplazo con algoritmo BSU");
			reemplazoBiggestSpaceUsed();
			break;
	}

}

bool cargar_valor(char * clave , char * valor){

	t_list * entradas_usar = list_create();

	int espacio_necesario = strlen(valor);
	int tamanio_valor = 0;

	// CALCULO CANTIDAD ENTRADAS
	float division = espacio_necesario / tamanio_entradas ;
	int intpart = (int)division;
	float decpart = division - intpart;

	if (decpart > 0.5){
		tamanio_valor = round(decpart);
	}else{
		tamanio_valor = round(decpart) + 1;
	}

	// DISPONIBILIDAD EN ENTRADAS
	int i = 0;
	int entrada_inicial= 0;
	int entrada_final = 0;

	for (i=0 ; i < list_size(lista_entradas) ; i++){

		char clave[cantidad_entradas];
		sprintf(clave, "%d", i);

		int usado = dictionary_get(tabla_entradas,clave);

		if(!usado){
			list_add( entradas_usar , i);
			//TODO
		}
	}

	if (entrada_inicial == entrada_final)
		return false;

}

void organizar_carga(){

	// DEPENDERA DEL TIPO DE ALMACENAMIENTO

	// EJEMPLOS
	char * clave1 = "messi";
	char * clave2 = "nico";

	char * valor1 = "jugador";
	char * valor2 = "tosco";

	// COMIENZO CON CLAVE1

	if(!cargar_valor(clave1 , valor1)){
		reemplazar_por_algoritmo();
		cargar_valor(clave1 , valor1)
	}

}

// INICIO DE PROCESO
int main(void) {

	print_header();
	create_log();
	loadConfig();
	log_inicial_consola();

	init_structs();
	load_dump_files();

	cargar_valor();

	//connect_with_coordinator();
	//send_example();


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

