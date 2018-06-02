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
	log_info(console_log, "\t Tamanio entradas: %d" , tamanio_entradas);
	log_info(console_log, "\t Cantidad de entradas: %d" , cantidad_entradas);

	for(int i=0 ; i < cantidad_entradas ; i++){

		log_info(console_log, "Se carga entrada:  %d" , i);

		t_entrada* entrada = malloc(sizeof(t_entrada));
		entrada->siguiente_instruccion = 0;
		entrada->tamanio = tamanio_entradas;
		entrada->valor = malloc(tamanio_entradas);

		list_add(lista_entradas, entrada);

		char clave[cantidad_entradas];
		sprintf(clave, "%d", i);

		// @param i  ->  identificador entrada
		// @param 0  ->  valor de uso de la entrada ( 0 o 1 ) por si o por no
		dictionary_put(tabla_entradas ,clave , 0);

		// nota -> Deberia poder obtener el valor directamente de la posicion del dictionario

	}

	log_info(console_log, "Finished building structures");

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

void init_structs(t_instance_init_values * init_struct){

	tabla_entradas = dictionary_create();
	tabla_claves = dictionary_create();
	lista_entradas = list_create();

	build_tabla_entradas();

	//show_structs();
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

	if(!perform_connection_handshake(coordinator_socket, instancia_setup.NOMBRE_INSTANCIA, REDIS_INSTANCE, console_log)){
		exit_program(EXIT_FAILURE);
	}

	void* init_buffer = malloc(INSTANCE_INIT_VALUES_SIZE);

	if (recv(coordinator_socket, init_buffer, INSTANCE_INIT_VALUES_SIZE, MSG_WAITALL) <= 0) {
		log_error(console_log, "Error receiving initialization response. Aborting execution.");
		free(init_buffer);
		exit_program(EXIT_FAILURE);
	}

	t_instance_init_values * init_struct = deserialize_init_instancia_message(init_buffer);

	log_info(console_log, "Sending OK ");

	// SEND OK

	init_structs(init_struct);
	free(init_struct);
}

bool existe_capacidad_valor(char * valor){
	// TODO
	return true;
}

//void reemplazar_por_algoritmo(){
//
//	switch(instancia_setup.ALGORITMO_REEMPLAZO){
//		case CIRC:
//			log_info(console_log, "Comienza reemplazo con algoritmo CIRCULAR");
//			reemplazoCircular();
//			break;
//		case LRU:
//			log_info(console_log, "Comienza reemplazo con algoritmo LAST RECENTLY USED");
//			reemplazoLeastRecentlyUsed();
//			break;
//		case BSU:
//			log_info(console_log, "Comienza reemplazo con algoritmo BSU");
//			reemplazoBiggestSpaceUsed();
//			break;
//	}
//
//}

t_list * cargar_valor(char * clave , char * valor){

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
	IP = i;

	for (i=0 ; i < list_size(lista_entradas) ; i++){

		char clave[cantidad_entradas];
		sprintf(clave, "%d", i);

		// VER DE CAMBIAR
		int usado = dictionary_get(tabla_entradas,clave);

		if(!usado){
			list_add( entradas_usar , i);

			if(list_size(entradas_usar) == tamanio_valor)
				break;
		}else{

			if(list_size(entradas_usar) > 0){
				// ESPACIO NO CONTIGUO
				list_clean(entradas_usar);
			}
		}
	}

	// CAMBIAR IP
	IP = i;

	return entradas_usar;

}

void carga_real (t_list * lista , char * clave , char * valor){

	int tamanio_lista = list_size(lista);

	log_info(console_log , "Cantidad de entradas a usar para el valor: d%" , tamanio_lista);

	int pos = 0;

	for (int a = 0 ; a < list_size(lista) ; a++){

		int pos  = list_get(lista , a);

		t_entrada  * entrada = list_get(lista_entradas , pos);

		if (tamanio_lista == 1 || ( (tamanio_lista - a ) == 1) ){
			memcpy(entrada->valor, valor, strlen(valor));
		}else{
			memcpy(entrada->valor, valor, entrada->tamanio);
		}

	}

	log_info(console_log , "se muestra el valor de las entradas");

	char * valor_total = malloc(strlen(valor));

	for(int b = 0 ; b < tamanio_lista ; b++){

		int pos  = list_get(lista , b);
		t_entrada  * entrada = list_get(lista_entradas , pos);

		memcpy(valor_total + pos, entrada->valor ,  entrada->tamanio);
		pos = pos + entrada->tamanio ;

	}

	log_info(console_log , "Valor: s%" , valor_total);

}

void organizar_carga(){

	// DEPENDERA DEL TIPO DE ALMACENAMIENTO

	// EJEMPLOS
	char * clave1 = "messi";
	char * clave2 = "nico";

	char * valor1 = "jugador";
	char * valor2 = "tosco";

	// COMIENZO CON CLAVE1

	t_list * lista = cargar_valor(clave1 , valor1);

	int tamanio_lista = list_size(lista);

	if(!tamanio_lista > 0){
		//reemplazar_por_algoritmo();
		//cargar_valor(clave1 , valor1);
		log_info(console_log , "no cargo bien");
	}else{
		carga_real(lista , clave1 , valor1);
		// SE DEBERIA ACTUALIZAR LOS DICCIONARIOS

		log_info(console_log , "cargo bien");
	}




}

// INICIO DE PROCESO
int main(void) {

	print_header();
	create_log();
	loadConfig();
	log_inicial_consola();

	connect_with_coordinator();

	// CICLO - INSTANCIA

	//load_dump_files();

	//organizar_carga();
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

