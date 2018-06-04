#include "Instancia.h"
#include <unistd.h> // Para close

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

void exit_program(int retVal) {
	instance_setup_destroy(&instance_setup);

	if (console_log != NULL)
		log_destroy(console_log);
	if (coordinator_socket != 0)
		close(coordinator_socket);

	if(memory_region != NULL)
		free(memory_region);

	if(occupied_memory_map != NULL){
		for(int i=0; i<number_of_entries; i++){
			if(occupied_memory_map[i] != NULL) free(occupied_memory_map[i]);
		}
		free(occupied_memory_map);
	}

	dictionary_destroy_and_destroy_elements(key_dictionary, entry_data_destroy);

	print_goodbye();
	exit(retVal);
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

	t_config *config = config_create(INSTANCE_CFG_FILE);

	if (config != NULL) {
		strcpy(instance_setup.IP_COORDINADOR, config_get_string_value(config, "IP_COORDINADOR"));
		instance_setup.PUERTO_COORDINADOR = config_get_int_value(config,"PUERTO_COORDINADOR");
		instance_setup.ALGORITMO_REEMPLAZO = config_get_int_value(config,"ALGORITMO_REEMPLAZO");
		strcpy(instance_setup.PUNTO_MONTAJE, config_get_string_value(config,"PUNTO_MONTAJE"));
		strcpy(instance_setup.NOMBRE_INSTANCIA, config_get_string_value(config,"NOMBRE_INSTANCIA"));
		instance_setup.INTERVALO_DUMP_SEGs = config_get_int_value(config, "INTERVALO_DUMP_SEGs");

		log_info(console_log, "COORDINADOR: IP: %s, PUERTO: %d",
					instance_setup.IP_COORDINADOR, instance_setup.PUERTO_COORDINADOR);

		switch (instance_setup.ALGORITMO_REEMPLAZO) {
		case CIRC:
			log_info(console_log, "Algoritmo de reemplazo: CIRC");
			perform_replacement_and_return_first_position = replace_circular;
			break;
		case LRU:
			log_info(console_log, "Algoritmo de reemplazo: LRU");
			perform_replacement_and_return_first_position = replace_circular; // TODO CAMBIAR!
			break;
		case BSU:
			log_info(console_log, "Algoritmo de planificacion: BSU");
			perform_replacement_and_return_first_position = replace_circular; // TODO CAMBIAR!
			break;
		}

		log_info(console_log, "Punto de montaje: %s",
				instance_setup.PUNTO_MONTAJE);
		log_info(console_log, "Nombre de la instancia: %s",
				instance_setup.NOMBRE_INSTANCIA);
		log_info(console_log, "Intervalo de dump en segundos: %d",
				instance_setup.INTERVALO_DUMP_SEGs);


		log_info(console_log, " Carga exitosa de archivo de configuracion");
		config_destroy(config);
	} else {
		log_error(console_log, "No se encontro la configuracion de la instancia");
		exit_program(EXIT_FAILURE);
	}
}


/*
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
		strcpy(entrada->valor , "PRUEBA VALOR");

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

	while(list_size(lista_entradas) > 0){

		t_entrada * entrada = list_get(lista_entradas , 1);

		log_info(console_log, "ENTRADA %d" , a);
		log_info(console_log, "ENTRADA %d" ,entrada->tamanio  );
		log_info(console_log, "ENTRADA %s", entrada->valor);

		list_remove(lista_entradas , 1);
		a++;
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
*/


void load_dump_files (){

	// TODO - 1. Se debe verificar que exista algun archivo y cargarlo a la tabla de entradas

}

bool wait_for_init_data(){
	void* buffer = malloc(INSTANCE_INIT_VALUES_SIZE);

	if (recv(coordinator_socket, buffer, INSTANCE_INIT_VALUES_SIZE, MSG_WAITALL) < INSTANCE_INIT_VALUES_SIZE) {
		log_error(console_log, "Error receiving handshake response. Aborting execution.");
		free(buffer);
		return false;
	}

	t_instance_init_values *init_values = deserialize_instance_init_values(buffer);

	entry_size = init_values->entry_size;
	number_of_entries = init_values->number_of_entries;
	storage_size = entry_size * number_of_entries;

	log_info(console_log, "Init values received from Coordinator. Entry size: %i, Number of entries: %i. Storage size: %i.",
			entry_size, number_of_entries, storage_size);

	free(buffer);
	free(init_values);

	return true;
}

bool setup_with_coordinator(){

	if(!send_connection_header(coordinator_socket, instance_setup.NOMBRE_INSTANCIA, REDIS_INSTANCE, console_log)){
		return false;
	}

	log_trace(console_log, "Handshake message sent. Waiting for response...");

	if(wait_for_init_data()){
		log_info(console_log, "Init data received from coordinator.");
		return true;
	} else {
		log_error(console_log, "Could not retrieve init data from coordinator!");
		return false;
	}

}

void connect_with_coordinator() {
	log_info(console_log, "Connecting to Coordinador.");
	coordinator_socket = connect_to_server(instance_setup.IP_COORDINADOR, instance_setup.PUERTO_COORDINADOR, console_log);
	if(coordinator_socket <= 0){
		exit_program(EXIT_FAILURE);
	}

	if(!setup_with_coordinator()){
		exit_program(EXIT_FAILURE);
	}
	log_info(console_log, "Successfully connected to Coordinador.");

}

t_memory_position* create_empty_memory_position(){
	t_memory_position* memory_pos = malloc(sizeof(t_memory_position));
	memory_pos->is_atomic = true;
	memory_pos->used = false;
	memory_pos->last_reference = 0;
	memory_pos->key[0] = '\0';

	return memory_pos;
}

void initialize_instance(){
	memory_region = malloc(storage_size);
	occupied_memory_map = malloc(number_of_entries * sizeof(t_memory_position*));
	for(int i = 0; i < number_of_entries; i++){
		occupied_memory_map[i] = create_empty_memory_position();
	}

	key_dictionary = dictionary_create();
}

void instance_setup_destroy(t_instance_setup* instance_setup){
	if(instance_setup->NOMBRE_INSTANCIA != NULL)
		free(instance_setup->NOMBRE_INSTANCIA);

	if(instance_setup->IP_COORDINADOR != NULL)
		free(instance_setup->IP_COORDINADOR);

	if(instance_setup->PUNTO_MONTAJE != NULL)
		free(instance_setup->PUNTO_MONTAJE);
}

void entry_data_destroy(t_entry_data* entry_data){
	if(entry_data != NULL)
		free(entry_data);
}

coordinator_operation_type_e wait_for_signal_from_coordinator(){
	void* buffer = malloc(COORDINATOR_OPERATION_HEADER_SIZE);

	if (recv(coordinator_socket, buffer, COORDINATOR_OPERATION_HEADER_SIZE, MSG_WAITALL) < COORDINATOR_OPERATION_HEADER_SIZE) {
		log_error(console_log, "Error receiving handshake response. Aborting execution.");
		free(buffer);
		exit_program(EXIT_FAILURE);
	}

	t_coordinator_operation_header* header = deserialize_coordinator_operation_header(buffer);

	coordinator_operation_type_e op_type = header->coordinator_operation_type;

	free(header);
	return op_type;
}

t_operation* receive_operation_from_coordinator(){
	void* buffer = malloc(OPERATION_REQUEST_SIZE);

	if (recv(coordinator_socket, buffer, OPERATION_REQUEST_SIZE, MSG_WAITALL) < OPERATION_REQUEST_SIZE) {
		log_error(console_log, "Error receiving operation from Coordinator. Aborting execution.");
		free(buffer);
		exit_program(EXIT_FAILURE);
	}

	t_operation_request* op_request = deserialize_operation_request(buffer);

	t_operation* operation = malloc(sizeof(t_operation));
	strcpy(operation->key, op_request->key);
	operation->operation_type = op_request->operation_type;
	operation->value_size = op_request->payload_size;
	operation->value = NULL;

	free(buffer);
	free(op_request);

	if(operation->value_size> 0){
		char* value_buffer = malloc(op_request->payload_size);

		if (recv(coordinator_socket, value_buffer, operation->value_size, MSG_WAITALL) < operation->value_size) {
			log_error(console_log, "Error receiving payload from Coordinator. Aborting execution.");
			free(value_buffer);
			free(operation);
			exit_program(EXIT_FAILURE);
		}
		operation->value = value_buffer;
	}

	return operation;
}

void send_response_to_coordinator(instance_status_e status, int payload_size){
	t_instance_response response;
	response.status = status;
	response.payload_size = payload_size;

	log_debug(console_log, "Sending response to cordinator: %i. Payload size: %i", status, payload_size);

	void* buffer = serialize_instance_response(&response);

	int result = send(coordinator_socket, buffer, INSTANCE_RESPONSE_SIZE, 0);

	free(buffer);

	if (result < INSTANCE_RESPONSE_SIZE) {
		log_error(console_log, "Could not send response to Coordinator. Aborting execution.");
		exit_program(EXIT_FAILURE);
	}
}

void send_stored_value_to_coordinator(char* stored_value, int value_length){
	log_debug(console_log, "Sending payload to coordinator.");

	int result = send(coordinator_socket, stored_value, value_length, 0);

	if (result < value_length) {
		log_error(console_log, "Could not send value to Coordinator. Aborting execution.");
		exit_program(EXIT_FAILURE);
	}
}

char* get_stored_value(char* operation_key){
	if(!dictionary_has_key(key_dictionary, operation_key)){
		return NULL;
	}

	t_entry_data* entry_data = (t_entry_data*)dictionary_get(key_dictionary, operation_key);
	char* stored_value = malloc(entry_data->size);

	int offset = entry_size * entry_data->first_position;

	log_debug(console_log, "Key: %s is stored at: %i. Size: %i. Calculated offset: %i",
			operation_key, entry_data->first_position, entry_data->size, offset);

	memcpy(stored_value, memory_region + offset, entry_data->size);

	return stored_value;
}

void handle_get(t_operation* operation){
	char* stored_value = get_stored_value(operation->key);
	if(stored_value == NULL){
		log_error(console_log, "Attempted GET on inexistent Key: %s.", operation->key);

		send_response_to_coordinator(INSTANCE_ERROR, 0);
		return;
	}

	log_info(console_log, "Retrieved value for key: %s. Value: %s", operation->key, stored_value);

	int value_size = strlen(stored_value);
	send_response_to_coordinator(INSTANCE_SUCCESS, value_size);
	send_stored_value_to_coordinator(stored_value, value_size);

	free(stored_value);
}

int slots_occupied_by(int value_size){
	int slots = value_size / entry_size;
	if(value_size % entry_size > 0) slots++;

	return slots;
}

void free_slot(int slot_index){
	t_memory_position* mem_pos = occupied_memory_map[slot_index];
	mem_pos->is_atomic = true;
	mem_pos->key[0] = '\0';
	mem_pos->last_reference = 0;
	mem_pos->used = false;
}

void set_in_same_place(t_entry_data* entry_data, t_operation* operation, int needed_slots, int used_slots){
	int offset = entry_data->first_position * entry_size;
	memcpy(memory_region + offset, operation->value, operation->value_size);
	entry_data->size = operation->value_size;

	int slots_to_free = used_slots - needed_slots;

	int slot_index = entry_data->first_position + needed_slots;
	while(slots_to_free > 0){
		free_slot(slot_index);
		slot_index++;
		slots_to_free--;
	}

}

// TODO: Repite codigo con set_in_same_place. Refactor!
void remove_key(char* key, t_entry_data* entry_data, int used_slots){
	int slot_index = entry_data->first_position;
	int slots_to_free = used_slots;
	while(slots_to_free > 0){
		free_slot(slot_index);
		slot_index++;
		slots_to_free--;
	}

	dictionary_remove_and_destroy(key_dictionary, key, entry_data_destroy);
}

/*
 * Checks the memory slots for available space to fit the required size.
 * Returns True if there is space enough to fit, either contiguous or not.
 * If threre is contiguous space available, first_slot will be set with the first
 * slot of that contiguous space. Otherwise, it will be set to -1.
 */
bool check_if_free_slots_available(int required_slots, int* first_slot){
	int total_free_slots = 0;
	int contiguous_free_slots = 0;
	int current_first = -1;
	int cursor;
	t_memory_position* mem_pos;

	for(cursor = 0; cursor < number_of_entries && contiguous_free_slots < required_slots; cursor++){
		mem_pos = occupied_memory_map[cursor];
		if(mem_pos->used){
			current_first = -1;
			contiguous_free_slots = 0;
		} else{
			total_free_slots++;
			if(current_first > 0){
				contiguous_free_slots++;
			} else {
				current_first = cursor;
				contiguous_free_slots = 1;
			}
		}
	}

	if(contiguous_free_slots == required_slots){
		*first_slot = current_first;
	}

	return total_free_slots >= required_slots;
}

void handle_set(t_operation* operation){
	int need_slots = slots_occupied_by(operation->value_size);

	// Check if key is already present
	if(dictionary_has_key(key_dictionary, operation->key)){
		t_entry_data* entry_data = (t_entry_data*)dictionary_get(key_dictionary, operation->key);
		int used_slots = slots_occupied_by(entry_data->size);

		if(need_slots <= used_slots){
			// If the new value fits in the previously reserved slots, use those slots
			set_in_same_place(entry_data, operation, need_slots, used_slots);
			send_response_to_coordinator(INSTANCE_SUCCESS, 0);
			return;
		} else{
			// If it does not fit, remove the key, free the slots and check if it fits somewhere else.
			remove_key(operation->key, entry_data, used_slots);
		}
	}

	// At this point the key is either new or it was removed because the new value did not fit.

	// verificar si hay espacio contiguo disponible
	// si hay espacio contiguo disponible, usarlo.
	// si no hay espacio contiguo, pero si espacio, hay que compactar.
	// sino, llamar al algoritmo de reemplazo.

	int first_slot; // This value is filled by the next call to check_if_free_slots_available.
	bool space_available = check_if_free_slots_available(need_slots, &first_slot);

	if(space_available) {
		if(first_slot < 0){
			// Need to compact because there is space available but it is not contiguous
			log_info(console_log, "There is space available but not contiguous to SET the value with size: %s. Need to compact.",
					operation->value_size);
			send_response_to_coordinator(INSTANCE_COMPACT, 0);
			return;
		}
	} else {
		first_slot = perform_replacement_and_return_first_position(operation->value_size);
	}

	// create the new key for the dictionary
	t_entry_data* entry_data = malloc(sizeof(t_entry_data));
	entry_data->first_position = first_slot;
	entry_data->size = operation->value_size;

	// mark the slots as used by this key
	t_memory_position* mem_pos;
	bool isAtomic = need_slots == 1;

	for(int pos = first_slot; pos < first_slot + need_slots; pos++){
		mem_pos = occupied_memory_map[pos];
		mem_pos->is_atomic = isAtomic;
		strcpy(mem_pos->key, operation->key);
		mem_pos->last_reference = 0; // TODO: FALTA SETEAR UNA VARIABLE CON EL NRO DE ITERACION
		mem_pos->used = true;
	}

	// memcpy the new value
	int offset = first_slot * entry_size;
	memcpy(memory_region + offset, operation->value, operation->value_size);

	// save the new key in the dictionaty
	dictionary_put(key_dictionary, operation->key, entry_data);

	send_response_to_coordinator(INSTANCE_SUCCESS, 0);
}

bool do_store(char* key, char* value){
	// TODO!
	return true;
}

void handle_store(t_operation* operation){
	char* stored_value = get_stored_value(operation->key);
	if(stored_value == NULL){
		log_error(console_log, "Attempted STORE on inexistent Key: %s.", operation->key);

		send_response_to_coordinator(INSTANCE_ERROR, 0);
		return;
	}

	log_info(console_log, "Retrieved value for key: %s. Value: %s", operation->key, stored_value);

	if(do_store(operation->key, stored_value)){
		send_response_to_coordinator(INSTANCE_SUCCESS, 0);
	} else {
		log_error(console_log, "Could not store key: %s, value: %s. Aborting execution.", operation->key, stored_value);
		exit_program(EXIT_FAILURE);
	}
}

void operation_destroy(t_operation* operation){
	if(operation != NULL){
		if(operation->value != NULL) free(operation->value);
		free(operation);
	}
}

void handle_operation(){
	log_debug(console_log, "Received key operation from Coordinator.");
	t_operation* operation = receive_operation_from_coordinator();

	switch(operation->operation_type){
		case GET:
			log_info(console_log, "Received GET for Key: %s", operation->key);
			handle_get(operation);
			break;
		case SET:
			log_info(console_log, "Received SET for Key: %s Value: %s", operation->key, operation->value);
			handle_set(operation);
			break;
		case STORE:
			log_info(console_log, "Received STORE for Key: %s", operation->key);
			handle_store(operation);
			break;
	}

	operation_destroy(operation);
}

void compact() {
	// TODO!
}

int replace_circular(unsigned int value_size){
	// TODO!
	return -1;
}

int main(void) {
	print_header();
	create_log();
	loadConfig();

	connect_with_coordinator();

	initialize_instance();

	load_dump_files();


	// 1. AL conectarse definir tamanio de entradas

	//Recibe sentencia
	//Extrae la clave
	//Identifica donde guardarlo. Si no hay espacio, le avisa al coordinador que tiene que compactar. Compacta
	//Guardar la clave
	//Envía el resultado al Coordinador

	coordinator_operation_type_e coordinator_operation_type;

	while(true){
		coordinator_operation_type = wait_for_signal_from_coordinator();

		switch(coordinator_operation_type){
		case  KEY_OPERATION:
			handle_operation();
			break;
		case COMPACT:
			compact();
			break;
		}
	}



	return 0;
}

