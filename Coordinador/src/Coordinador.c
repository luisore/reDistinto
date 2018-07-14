#include "Coordinador.h"
#include "libs/protocols.h"
#include <stdlib.h>
#include <commons/string.h>


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

	pthread_mutex_destroy(&mutex_planner_console);
	pthread_mutex_destroy(&mutex_principal);
	pthread_mutex_destroy(&mutex_all);
	pthread_mutex_destroy(&mutex_compaction);

	dictionary_destroy(key_instance_dictionary);

	printf("\n\t\e[31;1m FINALIZA COORDINADOR \e[0m\n");
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

void create_log_operations() {

	coordinador_log_operation = log_create("log_op.log", "[OPERATIONS]", false, LOG_LEVEL_TRACE);

	if (coordinador_log_operation == NULL) {
		printf(" FALLO - Creacion de Log de Operaciones");
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
		coordinador_config.NOMBRE_INSTANCIA = malloc(30);
		strcpy(coordinador_config.NOMBRE_INSTANCIA,
				config_get_string_value(config, "NOMBRE_INSTANCIA"));
		coordinador_config.PUERTO_ESCUCHA_CONEXIONES = config_get_int_value(	config, "PUERTO_ESCUCHA_CONEXIONES");
		coordinador_config.CANTIDAD_MAXIMA_CLIENTES = config_get_int_value(config,"CANTIDAD_MAXIMA_CLIENTES");
		coordinador_config.TAMANIO_COLA_CONEXIONES = config_get_int_value(config,"TAMANIO_COLA_CONEXIONES");
		coordinador_config.ALGORITMO_DISTRIBUCION = config_get_int_value(config,	"ALGORITMO_DISTRIBUCION");
		coordinador_config.CANTIDAD_ENTRADAS = config_get_int_value(config,"CANTIDAD_ENTRADAS");
		coordinador_config.TAMANIO_ENTRADA_BYTES = config_get_int_value(config,"TAMANIO_ENTRADA_BYTES");
		coordinador_config.RETARDO_MS = config_get_int_value(config,	"RETARDO_MS");
		coordinador_config.PUERTO_ESCUCHA_CONEXION_CONSOLA = config_get_int_value(config, "PUERTO_ESCUCHA_CONEXION_CONSOLA");
	}
	config_destroy(config);
}

void liberar_memoria() {
	if(connected_clients != NULL) list_destroy_and_destroy_elements(connected_clients, destroy_connected_client);
	if(connected_instances != NULL) list_destroy_and_destroy_elements(connected_instances, destroy_connected_client);
	if(server != NULL) tcpserver_destroy(server);
	if(coordinador_config.NOMBRE_INSTANCIA != NULL) free(coordinador_config.NOMBRE_INSTANCIA);
	if(distributor != NULL) distributor_destroy(distributor);
}

void log_inicial_consola() {


	log_info(coordinador_log, "Se muestran los datos del coordinador");

	switch (coordinador_config.ALGORITMO_DISTRIBUCION) {
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

	log_info(coordinador_log, "\tNombre de instancia: %s",	coordinador_config.NOMBRE_INSTANCIA);
	log_info(coordinador_log, "\tPuerto de escucha conexiones: %d",	coordinador_config.PUERTO_ESCUCHA_CONEXIONES);
	log_info(coordinador_log, "\tCantidad maxima de clientes: %d",	coordinador_config.CANTIDAD_MAXIMA_CLIENTES);
	log_info(coordinador_log, "\tTamanio cola conexiones: %d",	coordinador_config.TAMANIO_COLA_CONEXIONES);
	log_info(coordinador_log, "\tCantidad de entradas: %d",	coordinador_config.CANTIDAD_ENTRADAS);
	log_info(coordinador_log, "\tTamanio de entrada en bytes: %d", coordinador_config.TAMANIO_ENTRADA_BYTES);
	log_info(coordinador_log, "\tRetardo en milis: %d", coordinador_config.RETARDO_MS);
	log_info(coordinador_log, "\tPuerto de escucha conexion con consola de planificador : %d",	coordinador_config.PUERTO_ESCUCHA_CONEXION_CONSOLA);

}

void create_tcp_server(){

	pthread_mutex_lock(&mutex_all);

	connected_clients = list_create();
	connected_instances = list_create();
	instancia_actual=0;
	key_instance_dictionary =  dictionary_create();

	server = tcpserver_create(coordinador_config.NOMBRE_INSTANCIA, coordinador_log,
			coordinador_config.CANTIDAD_MAXIMA_CLIENTES,
			coordinador_config.TAMANIO_COLA_CONEXIONES,
			coordinador_config.PUERTO_ESCUCHA_CONEXIONES, true);

	if(server == NULL){
		log_error(coordinador_log, "Could not create TCP server. Aborting execution.");
		exit_program(EXIT_FAILURE);
	}

	pthread_mutex_unlock(&mutex_all);

}

void create_tcp_server_console(){

	pthread_mutex_lock(&mutex_all);

	server_planner_console = tcpserver_create("CONSOLE PLANNER", coordinador_log,
							1, 1,coordinador_config.PUERTO_ESCUCHA_CONEXION_CONSOLA, false);

	if(server_planner_console == NULL){
		log_error(coordinador_log, "Could not create TCP server for PLANNER CONSOLE. Aborting execution.");
		pthread_exit(0);
		exit_program(EXIT_FAILURE);
	}

	pthread_mutex_unlock(&mutex_all);
}

void before_tpc_server_cycle(tcp_server_t* server){
	// ACÁ DEBERÍA IR LA LÓGICA DE DISTRIBUCION
}


void actualize_instance_dictionary(t_connected_client * instancia){
	// Actualize dictionary connected value

	if( dictionary_size(key_instance_dictionary) > 0  ){

		void find_instance_and_actualize(char * key , t_dictionary_instance_struct * instance_structure){
			int compare_string = strcmp(instance_structure->instance->instance_name ,instancia->instance_name);

			if(compare_string == 0){
				instance_structure->isConnected = false;
			}
		}

		dictionary_iterator(key_instance_dictionary,find_instance_and_actualize);

	}

}

void remove_client(tcp_server_t* server, int socket_id){
	bool is_linked_to_socket(void* conn_client){
		t_connected_client* connected_client = (t_connected_client*)conn_client;
		return connected_client->socket_id == socket_id;
	};

	tcpserver_remove_client(server, socket_id);
	list_remove_and_destroy_by_condition(connected_clients, is_linked_to_socket, destroy_connected_client);
}

void remove_instance(tcp_server_t* server, int socket_id){
	bool is_linked_to_socket(void* conn_client){
		t_connected_client* connected_client = (t_connected_client*)conn_client;
		return connected_client->socket_id == socket_id;
	};

	tcpserver_remove_client(server, socket_id);
	t_connected_client* connected_client = list_remove_by_condition(connected_instances, is_linked_to_socket);

	if(connected_client == NULL){
		log_error(coordinador_log, "Cannot remove conected instance. There was no mapping for socket id: %i", socket_id);
	}

	actualize_instance_dictionary(connected_client);
	distributor_remove_instance(distributor, connected_client->instance_name);
	log_info(coordinador_log, "Disconnected Instance: %s", connected_client->instance_name);
	destroy_connected_client(connected_client);
}

t_operation_response * send_operation_to_planner(char * recurso, t_connected_client * planner, operation_type_e t){

	t_coordinator_operation_request e;
	strcpy(e.key, recurso);
	e.operation_type = t;

	void *buffer = serialize_coordinator_operation_request(&e);

	log_info(coordinador_log, "Sending operation request from ESI to PLANNER");

	int send_data = send(planner->socket_reference, buffer, COORDINATOR_OPERATION_REQUEST_SIZE, 0);

	if(send_data < COORDINATOR_OPERATION_REQUEST_SIZE){
		log_error(coordinador_log, "It was an Error trying to send instruction to Planner. Aborting conection");

		remove_client(server, planner->socket_id);
		free(buffer);
		t_operation_response *response = malloc(OPERATION_RESPONSE_SIZE);
		response->operation_result = OP_ERROR;
		return response;

	}

	free(buffer);

	int bytesReceived = 0;
	void *res_buffer = malloc(COORDINATOR_OPERATION_REQUEST_SIZE);

	bytesReceived = recv(planner->socket_reference, res_buffer, OPERATION_RESPONSE_SIZE, MSG_WAITALL);

	if (bytesReceived < OPERATION_RESPONSE_SIZE) {

		log_error(coordinador_log, "It was an Error trying to receive instruction from Planner. Aborting conecction");
		log_error(coordinador_log, "Bytes leidos: %d | Esperados: %d",
				bytesReceived, OPERATION_RESPONSE_SIZE);

		free(res_buffer);

		remove_client(server, planner->socket_id);

		t_operation_response *response = malloc(OPERATION_RESPONSE_SIZE);
		response->operation_result = OP_ERROR;
		return response;
	}
	t_operation_response *response =
				deserialize_operation_response(res_buffer);

	free(res_buffer);

	log_info(coordinador_log, "Operation status well received from PLANNER");
	log_info(coordinador_log, "PLANNER Response: %d", response->operation_result);

	return response;
}

// TODO: REFACTOR!!!
void on_server_accept(tcp_server_t* server, int client_socket, int socket_id){
	void *header_buffer = malloc(CONNECTION_HEADER_SIZE);

	int res = recv(client_socket, header_buffer, CONNECTION_HEADER_SIZE, MSG_WAITALL);
	if (res <= 0) {
		log_error(coordinador_log, "Error receiving handshake request from TCP Client!");
		remove_client(server, socket_id);
		free(header_buffer);
		return;
	}

	t_connection_header *connection_header = deserialize_connection_header(header_buffer);
	log_info(coordinador_log, "Received handshake from TCP Client: %s", connection_header->instance_name);
	free(header_buffer);
	switch (connection_header->instance_type){
	case REDIS_INSTANCE:
		distributor_add_instance(distributor, connection_header->instance_name, 0); // TODO: USE REAL SPACED USED BY INSTANCE
		send_message_instance(connection_header, client_socket, socket_id);
		break;
	default:
		send_message_clients(connection_header, client_socket, socket_id);
	}

	//TODO: Modularizar

	t_connected_client* connected_client = malloc(sizeof(t_connected_client));

	strcpy(&(connected_client->instance_name), connection_header->instance_name);
	connected_client->instance_type = connection_header->instance_type;
	connected_client->socket_id = socket_id;
	connected_client->socket_reference = client_socket;

	list_add(connected_clients, (void*)connected_client);

	if(connection_header->instance_type == REDIS_INSTANCE){
		// Add to intances list -> For algorithims
		list_add(connected_instances, (void*)connected_client);

		if( dictionary_size(key_instance_dictionary) > 0  ){

			void find_instance_and_actualize(char * key , t_dictionary_instance_struct * instance_structure){

				int compare_string = strcmp(instance_structure->instance->instance_name ,connected_client->instance_name);

				if(compare_string == 0){
					instance_structure->instance->socket_id = connected_client->socket_id;
					instance_structure->instance->socket_reference = connected_client->socket_reference;
				}

			};
			dictionary_iterator(key_instance_dictionary,find_instance_and_actualize);

		}

	}

	free(connection_header);
}

void send_message_instance(t_connection_header *connection_header, int client_socket, int socket_id){
	t_instance_init_values init_values_message;
	init_values_message.entry_size = coordinador_config.TAMANIO_ENTRADA_BYTES;
	init_values_message.number_of_entries = coordinador_config.CANTIDAD_ENTRADAS;
	void *init_value_instance_buffer = serialize_init_instancia_message(&init_values_message);

	if( send(client_socket, init_value_instance_buffer, INSTANCE_INIT_VALUES_SIZE, 0) != INSTANCE_INIT_VALUES_SIZE)
	{
		log_error(coordinador_log, "Could not send handshake acknowledge to TCP client.");
		remove_client(server, socket_id);
	} else {
		log_info(coordinador_log, "Successfully connected to TCP Client: %s", connection_header->instance_name);
	}
	free(init_value_instance_buffer);
}

void send_message_clients(t_connection_header *connection_header, int client_socket, int socket_id){
	t_ack_message ack_message;

	strcpy(ack_message.instance_name, coordinador_config.NOMBRE_INSTANCIA);
	void *ack_buffer = serialize_ack_message(&ack_message);

	if( send(client_socket, ack_buffer, ACK_MESSAGE_SIZE, 0) != ACK_MESSAGE_SIZE)
	{
		log_error(coordinador_log, "Could not send handshake acknowledge to TCP client.");
		remove_client(server, socket_id);
	} else {
		log_info(coordinador_log, "Successfully connected to TCP Client: %s", connection_header->instance_name);
	}

	free(ack_buffer);
}

void send_message_planner_console(t_connection_header *connection_header, int client_socket, int socket_id){

	t_ack_message ack_message;
	strcpy(&(ack_message.instance_name), coordinador_config.NOMBRE_INSTANCIA);
	void *ack_buffer = serialize_ack_message(&ack_message);

	if( send(client_socket, ack_buffer, ACK_MESSAGE_SIZE, 0) != ACK_MESSAGE_SIZE)
	{
		log_error(coordinador_log, "Could not send handshake acknowledge to TCP client.");
	} else {
		log_info(coordinador_log, "Successfully connected to TCP Client: %s", connection_header->instance_name);
	}

	free(ack_buffer);
}

t_connected_client* find_connected_instance(int socket_id){
	bool is_linked_to_socket(void* conn_instance){
		t_connected_client* connected_instance = (t_connected_client*)conn_instance;
		return connected_instance->socket_id == socket_id;
	};

	return list_find(connected_instances, is_linked_to_socket);
}

t_connected_client* find_connected_client(int socket_id){
	bool is_linked_to_socket(void* conn_client){
		t_connected_client* connected_client = (t_connected_client*)conn_client;
		return connected_client->socket_id == socket_id;
	};

	return list_find(connected_clients, is_linked_to_socket);
}

t_connected_client* find_connected_client_by_type(instance_type_e instance_type){
	bool is_linked_to_socket(void* conn_client){
		t_connected_client* connected_client = (t_connected_client*)conn_client;
		return connected_client->instance_type == instance_type;
	};

	return list_find(connected_clients, is_linked_to_socket);
}

void send_response_to_esi(int esi_socket, t_connected_client* client, operation_result_e op_result){
	t_operation_response op_response;
	op_response.operation_result = op_result;

	char* buffer = serialize_operation_response(&op_response);

	int result = send(esi_socket, buffer, OPERATION_RESPONSE_SIZE, 0);

	if (result < OPERATION_RESPONSE_SIZE) {
		log_error(coordinador_log, "Signal execute next to ESI failed for ID: %d");
		remove_client(server, client->socket_id); // TODO: IDEM ANTES, REFACTORIZAR
	}
	free(buffer);
}

t_connected_client* do_select_instance(char* key, bool simulated){
	char* instance_name;
	if(simulated){
		instance_name = distributor_select_instance(distributor, key);
	} else {
		instance_name = distributor_simulate_select_instance(distributor, key);
	}

	if(instance_name == NULL){
		log_error(coordinador_log, "Could not select an instance. Maybe they all were disconected...");
		return NULL;
	}

	bool find_instance_by_name(void* connected_client){
		return connected_client != NULL &&
				string_equals_ignore_case(instance_name, ((t_connected_client*)connected_client)->instance_name);
	}

	t_connected_client* client = list_find(connected_instances, find_instance_by_name);

	free(instance_name);
	return client;
}


t_connected_client* select_instance(char* key){
	return do_select_instance(key, false);
}

t_connected_client* simulate_select_instance(char* key){
	return do_select_instance(key, true);
}

void bind_key_to_instance(char* key, t_connected_client* instance){
	t_dictionary_instance_struct * instance_structure = malloc(sizeof(t_dictionary_instance_struct));
	instance_structure->instance = malloc(sizeof(t_connected_client));

	strcpy(instance_structure->instance->instance_name , instance->instance_name);
	instance_structure->instance->instance_type = instance->instance_type;
	instance_structure->instance->socket_id = instance->socket_id;
	instance_structure->instance->socket_reference = instance->socket_reference;

	instance_structure->storage=50 ;// HARDCODE TODO: QUE ES ESTO???
	instance_structure->isConnected = true;

	dictionary_put(key_instance_dictionary, key, instance_structure);
}

bool secure_instance_for_set_request(char* key){
	t_dictionary_instance_struct * instance_structure = (t_dictionary_instance_struct *) dictionary_get(key_instance_dictionary, key);

	if(instance_structure != NULL){
		// check if the instance is connected or if we have to reassign it
		if(instance_structure->isConnected){
			return true;
		} else {
			dictionary_remove(key_instance_dictionary,key);
		}
	}

	t_connected_client* selected_instance = select_instance(key);

	if(selected_instance == NULL){
		log_error(coordinador_log, "Could not secure an instance for the GET request. Maybe all disconected...");
		return false;
	}

	bind_key_to_instance(key, selected_instance);
	return true;
}

void handle_esi_get(t_connected_client* planner, t_operation_request* esi_request, t_connected_client* client, int socket){
	// Add key to instance dictionary.
	// Redistribute instances

	log_info(coordinador_log, "Handling GET from ESI: %s. Key: %s.", client->instance_name, esi_request->key);

	t_operation_response* cod_result = send_operation_to_planner(esi_request->key, planner, GET);

	//Si el planificador me dice que esta bloqueado y no puedo ejecutar esa operacion, no se la mando a la isntancia.
	if(cod_result->operation_result == OP_BLOCKED){
		send_response_to_esi(socket, client, cod_result->operation_result);
		free(cod_result);
		return;
	}

//	operation_result_e op_result;

//	if(secure_instance_for_get_request(esi_request->key)){
//		log_info(coordinador_log, "Successful GET from ESI: %s. Key: %s.", client->instance_name, esi_request->key);
//		op_result = OP_SUCCESS;
//
//	} else {
//		log_info(coordinador_log, "There was an error processing GET from ESI: %s. Key: %s.", client->instance_name, esi_request->key);
//		op_result = OP_ERROR;
//	}

	send_response_to_esi(socket, client, cod_result->operation_result);

	// OPERATION - KEY
	log_info(coordinador_log_operation, "GET - %s " , esi_request->key);

	free(cod_result);
}

t_connected_client* get_instance_connected_for_key(char* key){
	t_dictionary_instance_struct * instance_structure = (t_dictionary_instance_struct *) dictionary_get(key_instance_dictionary , key );

	if(instance_structure == NULL){
		log_error(coordinador_log, "There was no mapped instance for key: %s", key);
		return NULL;
	}
	if (!instance_structure->isConnected){
		log_warning(coordinador_log, "Mapped instance for key:%s (%s) is disconnected.", key, instance_structure->instance->instance_name);
		return NULL;
	}
	return instance_structure->instance;
}

operation_result_e perform_instance_store(t_operation_request* esi_request){
	t_connected_client* instance = get_instance_connected_for_key(esi_request->key);

	if(instance == NULL){
		return OP_ERROR;
	}

	if(!send_operation_header_to_instance(instance)){
		log_error(coordinador_log, "Failed to send STORE operation header to instance: %s", instance->instance_name);
		return OP_ERROR;
	}

	if (!send_store_operation(esi_request, STORE, instance)){
		log_error(coordinador_log, "Failed to send STORE operation to instance: %s", instance->instance_name);
		return OP_ERROR;
	}

	return  OP_SUCCESS;

}

void handle_esi_store(t_connected_client* planner, t_operation_request* esi_request, t_connected_client* client, int socket){
	log_info(coordinador_log, "Handling STORE from ESI: %s. Key: %s.", client->instance_name, esi_request->key);

	t_operation_response* cod_result = send_operation_to_planner(esi_request->key, planner, STORE);

	//Si el planificador me dice que esta bloqueado y no puedo ejecutar esa operacion, no se la mando a la isntancia.
	if(cod_result->operation_result == OP_BLOCKED){
		send_response_to_esi(socket, client, OP_BLOCKED);
		free(cod_result);
		return;
	}
	free(cod_result);

	operation_result_e op_result = perform_instance_store(esi_request);

	send_response_to_esi(socket, client, op_result);

	// OPERATION - KEY
	log_info(coordinador_log_operation, "STORE - %s " , esi_request->key);
}

char* receive_payload_from_esi(t_operation_request* esi_request, t_connected_client* client, int socket){
	char *payload = malloc(esi_request->payload_size);
	int result = recv( socket, payload, esi_request->payload_size, MSG_WAITALL);

	if (result < esi_request->payload_size) {
		log_error(coordinador_log, "Error trying to receive payload from ESI: %s. Bytes read: %d. Expected: %d",
				client->instance_name, result, esi_request->payload_size);

		remove_client(server, client->socket_id);

		free(payload);
		return NULL;

	}
	return payload;
}

bool send_set_operation_to_instance(t_operation_request* esi_request, t_connected_client* instance, char* payload){
	t_operation_request operation;
	strcpy(operation.key, esi_request->key);
	operation.operation_type = SET;
	operation.payload_size = esi_request->payload_size;

	void *buffer = serialize_operation_request(&operation);

	if(send(instance->socket_reference, buffer, OPERATION_REQUEST_SIZE, 0) != OPERATION_REQUEST_SIZE){
		// Conection to instance fails. Must be removed.
		log_warning(coordinador_log , "Error trying to send SET OPERATION to Instance: $s", instance->instance_name);
		remove_instance(server , instance->socket_id);
		free(buffer);
		return false;
	}
	free(buffer);

	if( send(instance->socket_reference, payload, operation.payload_size, 0) != operation.payload_size){
		log_warning(coordinador_log , "Error trying to send SET PAYLOAD to Instance: $s", instance->instance_name);
		remove_instance(server , instance->socket_id);
		return false;
	}

	log_info(coordinador_log, "Successfully sent SET %s %s to Instance: %s", esi_request->key, payload, instance->instance_name);
	return true;
}

bool do_send_set_operation(t_operation_request* esi_request, t_connected_client *instance , char * payload, bool retry){
	if(!send_operation_header_to_instance(instance)){
		log_error(coordinador_log, "Error sending SET operation header to Instance: %s.", instance->instance_name);
		return false;
	}

	if(!send_set_operation_to_instance(esi_request, instance, payload)){
		return false;
	}

	t_instance_response* response = receive_response_from_instance(instance);
	instance_status_e instance_status = response->status;
	free(response);

	if(instance_status == INSTANCE_COMPACT){
		if(retry){
			log_info(coordinador_log, "Compaction finished. Retrying SET %s %s operation on instance: %s.",
					esi_request->key, payload, instance->instance_name);
			return do_send_set_operation(esi_request, instance, payload, false);
		} else {
			log_error(coordinador_log, "Entered in a compaction loop with instance: %s. Removing instance.", instance->instance_name);
			remove_instance(server, instance->socket_id);
		}
	}

	return (instance_status == INSTANCE_SUCCESS);
}

bool send_set_operation(t_operation_request* esi_request, t_connected_client *instance , char * payload){
	return do_send_set_operation(esi_request, instance, payload, true);
}

operation_result_e perform_instance_set(t_operation_request* esi_request, char* payload){
	t_dictionary_instance_struct * instance_structure = (t_dictionary_instance_struct *) dictionary_get(key_instance_dictionary , esi_request->key );

	if(instance_structure == NULL ){
		log_error(coordinador_log, "No instance found for key: %s.", esi_request->key);
		return OP_ERROR;
	}

	if(!instance_structure->isConnected){
		log_error(coordinador_log, "Instance for key: %s found, but is disconnected: %s.", esi_request->key, instance_structure->instance->instance_name);
		return OP_ERROR;
	}

	if(!send_set_operation(esi_request, instance_structure->instance, payload)){
		log_error(coordinador_log, "Error sending SET operation to Instance: %s.", instance_structure->instance->instance_name);
		return OP_ERROR;
	}

	log_info(coordinador_log, "Successfully sent SET %s %s operation to instance: %s.",
			esi_request->key, payload, instance_structure->instance->instance_name);
	return OP_SUCCESS;
}

void handle_esi_set(t_connected_client* planner, t_operation_request* esi_request, t_connected_client* client, int socket){
	log_info(coordinador_log, "Handling SET from ESI: %s. Key: %s.", client->instance_name, esi_request->key);
	log_info(coordinador_log, "Waiting for payload from ESI");

	char *payload = receive_payload_from_esi(esi_request, client, socket);

	if(payload == NULL){
		return;
	}

	log_info(coordinador_log, "Retrieved payload for SET : %s ",payload);

	t_operation_response* cod_result = send_operation_to_planner(esi_request->key, planner, SET);
	operation_result_e op_result = cod_result->operation_result;
	free(cod_result);

	if(secure_instance_for_set_request(esi_request->key)){
		log_info(coordinador_log, "Successful SET from ESI: %s. Key: %s.", client->instance_name, esi_request->key);
		op_result = OP_SUCCESS;

	} else {
		log_info(coordinador_log, "There was an error processing SET from ESI: %s. Key: %s.", client->instance_name, esi_request->key);
		op_result = OP_ERROR;
	}


	if(op_result == OP_SUCCESS){
		op_result = perform_instance_set(esi_request, payload);
	}

	send_response_to_esi(socket, client, op_result);

	// OPERATION - KEY
	log_info(coordinador_log_operation, "SET %s %s" , esi_request->key, payload);
	free(payload);
}


void handle_esi_request(t_operation_request* esi_request, t_connected_client* client, int socket){

	t_connected_client* planner = find_connected_client_by_type(PLANNER);

	switch(esi_request->operation_type){
	case GET:
		handle_esi_get(planner, esi_request, client, socket);
		break;
	case STORE:
		handle_esi_store(planner, esi_request, client, socket);
		break;
	case SET:
		handle_esi_set(planner, esi_request, client, socket);
		break;
	}
}

char*  receive_value_from_instance(t_connected_client * instance , int payload_size){

	char* buffer = malloc(payload_size);

	if (recv(instance->socket_reference, buffer, payload_size, MSG_WAITALL) < payload_size) {

		log_warning(coordinador_log, "Instance Disconnected: %s", instance->instance_name);
		free(buffer);
		remove_client(server, instance->socket_id);
		remove_instance(server, instance->socket_id);
		return NULL;
	}

	return buffer;
}

void perform_instance_compaction(void* instance){
	t_coordinator_operation_header header;
	header.coordinator_operation_type = COMPACT;

	void* buffer_operation = serialize_coordinator_operation_header(&header);

	t_connected_client* the_instance = (t_connected_client*)instance;

	log_info(coordinador_log, "Initiating compact on instance: %s", the_instance->instance_name);

	if(send(the_instance->socket_reference, buffer_operation,COORDINATOR_OPERATION_HEADER_SIZE, MSG_WAITALL)<COORDINATOR_OPERATION_HEADER_SIZE){
		//Esto es por si se cae alguna instancia actualizo las correspondientes estructuras.
		log_error(coordinador_log, "Could not send message COMPACT to Instance: %s. Remove Instance.", the_instance->instance_name);
		free(buffer_operation);
		pthread_mutex_lock(&mutex_compaction);
		remove_instance(server , the_instance->socket_id);
		pthread_mutex_unlock(&mutex_compaction);
		sem_post(&compact_semaphore);
		pthread_exit(0);
	}

	free(buffer_operation);

	// Espero respuesta de Instancia

	void* response_buffer = malloc(INSTANCE_RESPONSE_SIZE);

	if(recv(the_instance->socket_reference,response_buffer, INSTANCE_RESPONSE_SIZE, MSG_WAITALL)<INSTANCE_RESPONSE_SIZE){
		log_error(coordinador_log, "Could not receive COMPACT response from instance: %s", the_instance->instance_name);
		pthread_mutex_lock(&mutex_compaction);
		remove_instance(server, the_instance->socket_id);
		pthread_mutex_unlock(&mutex_compaction);
		free(response_buffer);
		sem_post(&compact_semaphore);
		pthread_exit(0);

	}

	t_instance_response* instance_response = deserialize_instance_response(response_buffer);
	instance_status_e instance_status = instance_response->status;
	free(instance_response);

	if(instance_status == INSTANCE_SUCCESS){
		log_info(coordinador_log, "Successfully compacted on instance: %s", the_instance->instance_name);
	} else {
		log_error(coordinador_log, "Instance: %s returned error on compact. Removing instance.", the_instance->instance_name);
		pthread_mutex_lock(&mutex_compaction);
		remove_instance(server, the_instance->socket_id);
		pthread_mutex_unlock(&mutex_compaction);
	}

	sem_post(&compact_semaphore);
	pthread_exit(0);
}

void wait_for_compaction_thread(void* thread){
	sem_wait(&compact_semaphore);
}

void do_synchronized_compaction(){
	t_list* compaction_threads = list_create();

	void create_instance_thread(void* instance){
		pthread_t* thread = malloc(sizeof(pthread_t));
		pthread_create(thread, NULL, perform_instance_compaction, instance);
		list_add(compaction_threads, thread);
	}

	// TODO: MUTEX!
	list_iterate(connected_instances, create_instance_thread);

	list_iterate(compaction_threads, wait_for_compaction_thread);

	//list_destroy_and_destroy_elements(compaction_threads, free);
}

t_instance_response * receive_response_from_instance(t_connected_client * instance ){

	void* buffer = malloc(INSTANCE_RESPONSE_SIZE);

	if (recv(instance->socket_reference, buffer, INSTANCE_RESPONSE_SIZE, MSG_WAITALL) < INSTANCE_RESPONSE_SIZE) {

		log_warning(coordinador_log, "Instance Disconnected: %s", instance->instance_name);
		free(buffer);
		remove_client(server, instance->socket_id);
		remove_instance(server, instance->socket_id);
		return false;
	}

	t_instance_response* response = deserialize_instance_response(buffer);
	free(buffer);

	// update space used for LSU algorithm
	distributor_update_space_used(distributor, instance->instance_name, response->space_used);

	switch(response->status){
	case INSTANCE_SUCCESS:
		log_info(coordinador_log, "Receive status from Instance - SUCCESS");
		break;
	case INSTANCE_ERROR:
		log_error(coordinador_log, "Receive status from Instance - ERROR");
		break;
	case INSTANCE_COMPACT:
		log_info(coordinador_log , "NEED TO COMPACT - STARTING COMPACT ALGORITHIM");
		do_synchronized_compaction();
		break;
	}

	return response;
}

bool send_operation_header_to_instance( t_connected_client * instance){

	t_coordinator_operation_header header;
	header.coordinator_operation_type = KEY_OPERATION;

	void *init_value_instance_buffer = serialize_coordinator_operation_header(&header);

	log_info(coordinador_log , "Attemting to send OPERATION to Instance");

	if( send(instance->socket_reference, init_value_instance_buffer, COORDINATOR_OPERATION_HEADER_SIZE, 0) != COORDINATOR_OPERATION_HEADER_SIZE){

		// Conection to instance fails. Must be removed and replanify all instances.
		log_warning(coordinador_log , "It was an error trying to send OPERATION to an Instance. Aborting execution");
		remove_instance(server,instance->socket_id);

		// Verify free
		free(instance);
		free(init_value_instance_buffer);
		return false;

	}

	log_info(coordinador_log , "Operation request sended succesful to Instance");

	free(init_value_instance_buffer);

	return true;
}

bool send_store_operation(t_operation_request* esi_request, operation_type_e operation_type, t_connected_client *instance){

	t_operation_request operation;
	strcpy(operation.key, esi_request->key);
	operation.operation_type = operation_type;
	operation.payload_size = 0;

	bool response_status;

	log_info(coordinador_log , "Attemting to send STORE OPERATION to Instance");

	void *buffer = serialize_operation_request(&operation);

	if(send(instance->socket_reference, buffer, OPERATION_REQUEST_SIZE, 0) != OPERATION_REQUEST_SIZE){

		// Conection to instance fails. Must be removed and replanify all instances.
		log_warning(coordinador_log , "It was an error trying to send STORE OPERATION to an Instance. Aborting execution");
		remove_client(server,instance->socket_id);
		remove_instance(server , instance->socket_id);
		// Verify free
		free(instance);
		free(buffer);
		return false;
	}else{

		t_instance_response * response = receive_response_from_instance(instance);

		if(response->status == INSTANCE_COMPACT){
			// Compact was made - REDO

			// VER QUE INSTANCIA ESTE LEVANTADO
			if(find_connected_instance(instance->socket_id)==NULL){
				log_error(coordinador_log, "La instancia no existe - ERROR");
				response_status = false;
			}else{
				response_status = send_store_operation(esi_request , operation_type , instance);
			}
		}else{
			response_status = (response->status == INSTANCE_SUCCESS);
		}
		free(response);

	}
	free(buffer);

	return response_status;

}

t_instance_response *  send_get_operation( char * key ,t_connected_client *instance){

	t_operation_request operation;
	strcpy(operation.key, key);
	operation.operation_type = GET;
	operation.payload_size = 0;

	t_instance_response * response = malloc(INSTANCE_RESPONSE_SIZE);


	log_info(coordinador_log , "Attemting to send GET OPERATION to Instance");

	void *buffer = serialize_operation_request(&operation);

	if(send(instance->socket_reference, buffer, OPERATION_REQUEST_SIZE, 0) != OPERATION_REQUEST_SIZE){

		// Conection to instance fails. Must be removed and replanify all instances.
		log_warning(coordinador_log , "It was an error trying to send GET OPERATION to an Instance. Aborting execution");
		remove_client(server,instance->socket_id);
		remove_instance(server , instance->socket_id);
		// Verify free
		free(instance);
		return false;
	}else{

		response = receive_response_from_instance(instance);

	}
	free(buffer);

	return response;
}

void handle_esi_read(t_connected_client* client, int socket){
	char* buffer = malloc(OPERATION_REQUEST_SIZE);

	if (recv(socket, buffer, OPERATION_REQUEST_SIZE, MSG_WAITALL) < OPERATION_REQUEST_SIZE) {
		log_warning(coordinador_log, "ESI Disconnected: %s", client->instance_name);
		free(buffer);
		remove_client(server, client->socket_id); //TODO: NO HACE FALTA EL FIND PORQUE YA LO TENGO. SE PUEDE MEJORAR
		return;
	}

	t_operation_request* esi_request = deserialize_operation_request(buffer);

	handle_esi_request(esi_request, client, socket);

	free(esi_request);
	free(buffer);
}

void planner_disconected(int socket_id){
	log_warning(coordinador_log , "PLANNER has disconnected");
	remove_client(server,socket_id );
}

void instance_disconected(int socket_id){

	if(!flag_get){
		log_warning(coordinador_log , "INSTANCE has disconnected");
		remove_instance(server,socket_id );
	}else{
		flag_get = false;
	}

}

void on_server_read(tcp_server_t* server, int client_socket, int socket_id){

	// Verifico que proceso estoy leyendo:
	t_connected_client* client = find_connected_client(socket_id);

	if(client == NULL){
		// TODO: VER QUE HACEMOS! CLIENTE INVALIDO, no deberia pasar nunca
		return;
	}
	pthread_mutex_lock(&mutex_all);
	switch(client->instance_type){
	case ESI:
		handle_esi_read(client, client_socket);
		break;
	case REDIS_INSTANCE:
		instance_disconected(client->socket_id);
		break;
	case PLANNER:
		planner_disconected(client->socket_id);
		break;
	case COORDINATOR:
		break;
	}
	pthread_mutex_unlock(&mutex_all);

}

void on_server_command(tcp_server_t* server){

}


void destroy_connected_client(t_connected_client* connected_client){
	free(connected_client);
}

 char * retrieve_instance_value(char * key ,status_response_from_coordinator * response){

	 char * value;

	 t_dictionary_instance_struct * instance_structure = dictionary_get(key_instance_dictionary , key ); // CAMBIAR CUANDO SEBAS ARME LA ESTRUCTURA

	if( dictionary_size(key_instance_dictionary) > 0 && instance_structure != NULL ){

		strcpy(response->nombre_intancia_actual , instance_structure->instance->instance_name);
		strcpy(response->nombre_intancia_posible , "NO_VALOR");

		// Send GET OPERATION to Instance to retrieve value.

		if(!send_operation_header_to_instance(instance_structure->instance)){
			response->payload_valor_size = 0;
		}else{

			t_instance_response * response_instance = send_get_operation(key,instance_structure->instance);


			if(response_instance->status == INSTANCE_SUCCESS || response_instance->status == INSTANCE_COMPACT){

				value = malloc(response_instance->payload_size);
				value = receive_value_from_instance(instance_structure->instance , response_instance->payload_size);

				response->payload_valor_size = response_instance->payload_size + 1;
			}else{
				response->payload_valor_size = 0;
			}

		}

	}else{
		strcpy(response->nombre_intancia_actual ,"NO_VALOR");

		t_connected_client* simulated_instance = simulate_select_instance(key);
		strcpy(response->nombre_intancia_posible , simulated_instance->instance_name);
		response->payload_valor_size = 0;

	}

	return value;

}

void handle_planner_console_request(char * key , int planner_socket){

	log_info(coordinador_log , "CONSOLE_PLANNER: Receive key from console: %s" , key );
	log_info(coordinador_log , "Retrieving value from INSTANCE");



	status_response_from_coordinator *response =  malloc(STATUS_RESPONSE_FROM_COORDINATOR);

	char * key_value =  retrieve_instance_value(key , response);

	log_info(coordinador_log , "Sending status_struct to PLANNER");

	void *buffer = serialize_status_response_from_coordinator(response);

	int send_data = send(planner_socket, buffer, STATUS_RESPONSE_FROM_COORDINATOR, 0);

	if(send_data < STATUS_RESPONSE_FROM_COORDINATOR){
		log_error(coordinador_log, "It was an Error trying to send status_response to Planner. Aborting conection");
		tcpserver_remove_client(server_planner_console, planner_socket);
		free(buffer);
		return;
	}

	// Sends value if exists

	if (response->payload_valor_size > 0){
		log_info(coordinador_log , "Sending explicit value from associated key");

		int send_value = send(planner_socket, key_value,response->payload_valor_size, 0);

		if(send_value < response->payload_valor_size){
			log_error(coordinador_log, "It was an Error trying to send payload value to Planner. Aborting conection");
			tcpserver_remove_client(server_planner_console, planner_socket);
			free(buffer);
			return;
		}
	}

	log_info(coordinador_log , "Sended OK.");

	free(buffer);

}


void server_planner_console_accept(tcp_server_t* server, int client_socket, int socket_id){

	void *header_buffer = malloc(CONNECTION_HEADER_SIZE);

	int res = recv(client_socket, header_buffer, CONNECTION_HEADER_SIZE, MSG_WAITALL);
	if (res <= 0) {
		log_error(coordinador_log, "Error receiving handshake request from PLANNER CONSOLE");
		tcpserver_remove_client(server_planner_console, socket_id);
		free(header_buffer);
		return;
	}

	t_connection_header *connection_header = deserialize_connection_header(header_buffer);


	send_message_planner_console(connection_header, client_socket, socket_id);

	free(header_buffer);
	free(connection_header);
}

void server_planner_console_read(tcp_server_t* server, int client_socket, int socket_id){

	// First must receive key_size

	pthread_mutex_lock(&mutex_all);

	int key_size;

	if (recv(client_socket, &key_size,sizeof(key_size), MSG_WAITALL) == -1) {

		log_error(coordinador_log, "CONSOLE_PLANNER: Cannot receive key_size");
		tcpserver_remove_client(server_planner_console, socket_id);
		return;

	}
	log_info(coordinador_log , "Receive key size. Attempting to receive key_value");

	// Define size of key.
	char* key_buffer = malloc(key_size);

	if (recv(client_socket, key_buffer, key_size, MSG_WAITALL) < key_size) {

		log_error(coordinador_log, "CONSOLE_PLANNER: Cannot receive key");
		free(key_buffer);
		tcpserver_remove_client(server_planner_console, socket_id);
		return;

	}

	handle_planner_console_request(key_buffer , client_socket );

	free(key_buffer);

	flag_get = true;

	pthread_mutex_unlock(&mutex_all);
}


void coordinate_planner_console(){
	create_tcp_server_console();
	tcpserver_run(server_planner_console, before_tpc_server_cycle, server_planner_console_accept, server_planner_console_read, on_server_command);
	pthread_exit(0);
}

void coordinate_principal_process(){
	create_tcp_server();
	tcpserver_run(server, before_tpc_server_cycle, on_server_accept, on_server_read, on_server_command);
	pthread_exit(0);

}

void create_distributor(){
	distributor = distributor_init(coordinador_config.ALGORITMO_DISTRIBUCION, coordinador_log);
}


int main(int argc, char **argv) {
	if (argc > 1 && strcmp(argv[1], "-runTests") == 0){
		run_tests();
		return 0;
	}

	print_header();
	create_log();
	create_log_operations();
	loadConfig();
	create_distributor();
	log_inicial_consola();


	pthread_mutex_init(&mutex_all, NULL);
	pthread_mutex_init(&mutex_compaction, NULL);
	sem_init(&compact_semaphore, 0, 0);

	// HILO CONSOLA PLANIFICADOR
	pthread_mutex_init(&mutex_planner_console, NULL);
	pthread_create(&thread_planner_console, NULL, (void*) coordinate_planner_console, NULL);

	// HILO PRINCIPAL
	pthread_mutex_init(&mutex_principal, NULL);
	pthread_create(&thread_principal, NULL, (void*) coordinate_principal_process, NULL);

	pthread_join(thread_planner_console, NULL);
	pthread_join(thread_principal, NULL);

	print_goodbye();
	exit_program(EXIT_SUCCESS);

	return 0;

}
