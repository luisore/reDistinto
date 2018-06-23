#include "Coordinador.h"
#include "libs/protocols.h"
#include <stdlib.h>


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

void loadConfig() {

	log_info(coordinador_log, " Cargan datos del archivo de configuracion");

	t_config *config = config_create(PATH_FILE_NAME);

	if (config == NULL) {
		log_error(coordinador_log,
				"FALLO - No se encontro la configuracion del log");
		exit_program(EXIT_FAILURE);
	}

	if (config != NULL) {
		coordinador_setup.NOMBRE_INSTANCIA = string_duplicate(config_get_string_value(config,"NOMBRE_INSTANCIA"));
		coordinador_setup.PUERTO_ESCUCHA_CONEXIONES = config_get_int_value(	config, "PUERTO_ESCUCHA_CONEXIONES");
		coordinador_setup.CANTIDAD_MAXIMA_CLIENTES = config_get_int_value(config,"CANTIDAD_MAXIMA_CLIENTES");
		coordinador_setup.TAMANIO_COLA_CONEXIONES = config_get_int_value(config,"TAMANIO_COLA_CONEXIONES");
		coordinador_setup.ALGORITMO_DISTRIBUCION = config_get_int_value(config,	"ALGORITMO_DISTRIBUCION");
		coordinador_setup.CANTIDAD_ENTRADAS = config_get_int_value(config,"CANTIDAD_ENTRADAS");
		coordinador_setup.TAMANIO_ENTRADA_BYTES = config_get_int_value(config,"TAMANIO_ENTRADA_BYTES");
		coordinador_setup.RETARDO_MS = config_get_int_value(config,	"RETARDO_MS");
		coordinador_setup.PUERTO_ESCUCHA_CONEXION_CONSOLA = config_get_int_value(	config, "PUERTO_ESCUCHA_CONEXION_CONSOLA");
	}
	config_destroy(config);
}

void liberar_memoria() {
	if(connected_clients != NULL) list_destroy_and_destroy_elements(connected_clients, destroy_connected_client);
	if(connected_instances != NULL) list_destroy_and_destroy_elements(connected_instances, destroy_connected_client);
	if(server != NULL) tcpserver_destroy(server);
	if(coordinador_setup.NOMBRE_INSTANCIA != NULL) free(coordinador_setup.NOMBRE_INSTANCIA);
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

	log_info(coordinador_log, "\tNombre de instancia: %s",	coordinador_setup.NOMBRE_INSTANCIA);
	log_info(coordinador_log, "\tPuerto de escucha conexiones: %d",	coordinador_setup.PUERTO_ESCUCHA_CONEXIONES);
	log_info(coordinador_log, "\tCantidad maxima de clientes: %d",	coordinador_setup.CANTIDAD_MAXIMA_CLIENTES);
	log_info(coordinador_log, "\tTamanio cola conexiones: %d",	coordinador_setup.TAMANIO_COLA_CONEXIONES);
	log_info(coordinador_log, "\tCantidad de entradas: %d",	coordinador_setup.CANTIDAD_ENTRADAS);
	log_info(coordinador_log, "\tTamanio de entrada en bytes: %d", coordinador_setup.TAMANIO_ENTRADA_BYTES);
	log_info(coordinador_log, "\tRetardo en milis: %d", coordinador_setup.RETARDO_MS);
	log_info(coordinador_log, "\tPuerto de escucha conexion con consola de planificador : %d",	coordinador_setup.PUERTO_ESCUCHA_CONEXION_CONSOLA);

}

void create_tcp_server(){

	pthread_mutex_lock(&mutex_all);

	connected_clients = list_create();
	connected_instances = list_create();
	instancia_actual=0;

	server = tcpserver_create(coordinador_setup.NOMBRE_INSTANCIA, coordinador_log,
			coordinador_setup.CANTIDAD_MAXIMA_CLIENTES,
			coordinador_setup.TAMANIO_COLA_CONEXIONES,
			coordinador_setup.PUERTO_ESCUCHA_CONEXIONES, true);

	if(server == NULL){
		log_error(coordinador_log, "Could not create TCP server. Aborting execution.");
		exit_program(EXIT_FAILURE);
	}

	pthread_mutex_unlock(&mutex_all);

}

void create_tcp_server_console(){

	pthread_mutex_lock(&mutex_all);

	server_planner_console = tcpserver_create("CONSOLE PLANNER", coordinador_log,
							1,
							1,
							coordinador_setup.PUERTO_ESCUCHA_CONEXION_CONSOLA, true);

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

void remove_client(server, socket_id){
	bool is_linked_to_socket(void* conn_client){
		t_connected_client* connected_client = (t_connected_client*)conn_client;
		return connected_client->socket_id == socket_id;
	};

	tcpserver_remove_client(server, socket_id);
	list_remove_and_destroy_by_condition(connected_clients, is_linked_to_socket, destroy_connected_client);
}

void remove_instance(server, socket_id){
	bool is_linked_to_socket(void* conn_client){
		t_connected_client* connected_client = (t_connected_client*)conn_client;
		return connected_client->socket_id == socket_id;
	};

	tcpserver_remove_client(server, socket_id);
	list_remove_and_destroy_by_condition(connected_instances, is_linked_to_socket, destroy_connected_client);
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

	log_info(coordinador_log, "Operation status well received from PLANNER");
	log_info(coordinador_log, "PLANNER Response: %d", response->operation_result);
	return response;
}

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
	}

}

void send_message_instance(t_connection_header *connection_header, int client_socket, int socket_id){
	t_instance_init_values init_values_message;
			init_values_message.entry_size = coordinador_setup.TAMANIO_ENTRADA_BYTES;
			init_values_message.number_of_entries = coordinador_setup.CANTIDAD_ENTRADAS;
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
	strcpy(ack_message.instance_name, coordinador_setup.NOMBRE_INSTANCIA);
	void *ack_buffer = serialize_ack_message(&ack_message);

	if( send(client_socket, ack_buffer, ACK_MESSAGE_SIZE, 0) != ACK_MESSAGE_SIZE)
	{
		log_error(coordinador_log, "Could not send handshake acknowledge to TCP client.");
		remove_client(server, socket_id);
	} else {
		log_info(coordinador_log, "Successfully connected to TCP Client: %s", connection_header->instance_name);
	}

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

void handle_esi_request(t_operation_request* esi_request, t_connected_client* client, int socket){

	t_connected_client* planner = find_connected_client_by_type(PLANNER);
	t_operation_response *cod_result;

	// Select an instance based on the selected algorithim.
	t_connected_client * instance = select_instancia();

	switch(esi_request->operation_type){
	case GET:
		log_info(coordinador_log, "Handling GET from ESI: %s. Key: %s.", client->instance_name, esi_request->key);

		cod_result = send_operation_to_planner(esi_request->key, planner, GET);

//		if(!send_operation_to_instance(instance)){
//			cod_result->operation_result = OP_ERROR;
//		}else{
//
//			if(!send_get_operation(esi_request, esi_request->operation_type, instance)){
//				cod_result->operation_result = OP_ERROR;
//			}
//		}

		send_response_to_esi(socket, client, cod_result->operation_result);

		break;

	case STORE:
		log_info(coordinador_log, "Handling STORE from ESI: %s. Key: %s.", client->instance_name, esi_request->key);

		cod_result =  send_operation_to_planner(esi_request->key, planner, STORE);

		if(!send_operation_to_instance(instance)){
			cod_result->operation_result = OP_ERROR;
		}else{

			if(!send_store_operation(esi_request, esi_request->operation_type, instance)){
				cod_result->operation_result = OP_ERROR;
			}
		}

		send_response_to_esi(socket, client, cod_result->operation_result);

		break;
	case SET:
		log_info(coordinador_log, "Handling SET from ESI: %s. Key: %s.", client->instance_name, esi_request->key);
		log_info(coordinador_log, "Waiting for payload from ESI");

		char * payload_for_intance  = malloc(esi_request->payload_size);
		int result = recv( socket, payload_for_intance, esi_request->payload_size, MSG_WAITALL);

		if (result < esi_request->payload_size) {

			// On receive error

			log_error(coordinador_log, "It was an Error trying to receive payload from ESI. Aborting conection");
			log_error(coordinador_log, "Bytes leidos: %d | Esperados: %d",
					result, esi_request->payload_size);

			remove_client(server, client->socket_id);

			// TODO: Figure out how to send planner that ESI has stop working;

		}else{

			log_info(coordinador_log, "Retrieving value from SET : %s ",payload_for_intance);

			cod_result = send_operation_to_planner(esi_request->key, planner, SET);

			if(!send_operation_to_instance(instance)){
				cod_result->operation_result = OP_ERROR;
			}else{

				if(!send_set_operation(esi_request, esi_request->operation_type, instance ,payload_for_intance )){
					cod_result->operation_result = OP_ERROR;
				}
			}

		}

		send_response_to_esi(socket, client, cod_result->operation_result);
		break;
	}
}

bool receive_value_from_instance(t_connected_client * instance , int payload_size){

	char* buffer = malloc(payload_size);

	if (recv(instance->socket_reference, buffer, payload_size, MSG_WAITALL) < payload_size) {

		log_warning(coordinador_log, "Instance Disconnected: %s", instance->instance_name);
		free(buffer);
		remove_client(server, instance->socket_id);
		return false;
	}

	// Doesn't matter the receive value
	return true;
}

bool receive_response_from_instance(t_connected_client * instance ){

	void* buffer = malloc(INSTANCE_RESPONSE_SIZE);

	if (recv(instance->socket_reference, buffer, INSTANCE_RESPONSE_SIZE, MSG_WAITALL) < INSTANCE_RESPONSE_SIZE) {

		log_warning(coordinador_log, "Instance Disconnected: %s", instance->instance_name);
		free(buffer);
		remove_client(server, instance->socket_id);
		return false;
	}

	t_instance_response * response = deserialize_instance_response(buffer);

	switch(response->status){
	case INSTANCE_SUCCESS:
		log_info(coordinador_log, "Receive status from Instance - SUCCESS");
		free(response);
		return true;
		break;
	case INSTANCE_ERROR:
		log_error(coordinador_log, "Receive status from Instance - ERROR");
		free(response);
		return false;
		break;
	case INSTANCE_COMPACT:
		//TODO
		log_warning(coordinador_log, "Receive status from Instance - COMPACT");
		log_info(coordinador_log , "NEED TO COMPACT - STARTING COMPACT ALGORITHIM");
		free(response);
		return true;
		break;
	}
	return false;
}

bool send_operation_to_instance( t_connected_client * instance){

	t_coordinator_operation_header header;
	header.coordinator_operation_type = KEY_OPERATION;

	void *init_value_instance_buffer = serialize_coordinator_operation_header(&header);

	log_info(coordinador_log , "Attemting to send OPERATION to Instance");

	if( send(instance->socket_reference, init_value_instance_buffer, COORDINATOR_OPERATION_HEADER_SIZE, 0) != COORDINATOR_OPERATION_HEADER_SIZE){

		// Conection to instance fails. Must be removed and replanify all instances.
		log_warning(coordinador_log , "It was an error trying to send OPERATION to an Instance. Aborting execution");
		remove_client(server,instance->socket_id);

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

	bool response_status = false;
	bool value_instance = false;

	log_info(coordinador_log , "Attemting to send STORE OPERATION to Instance");

	void *buffer = serialize_operation_request(&operation);

	if(send(instance->socket_reference, buffer, OPERATION_REQUEST_SIZE, 0) != OPERATION_REQUEST_SIZE){

		// Conection to instance fails. Must be removed and replanify all instances.
		log_warning(coordinador_log , "It was an error trying to send STORE OPERATION to an Instance. Aborting execution");
		remove_client(server,instance->socket_id);

		// Verify free
		free(instance);
		return false;
	}else{

		response_status = receive_response_from_instance(instance);

	}
	free(buffer);

	return response_status;

}


bool send_get_operation(t_operation_request* esi_request, operation_type_e operation_type, t_connected_client *instance){

	t_operation_request operation;
	strcpy(operation.key, esi_request->key);
	operation.operation_type = operation_type;
	operation.payload_size = 0;

	bool response_status = false;
	bool value_instance = false;

	log_info(coordinador_log , "Attemting to send GET OPERATION to Instance");

	void *buffer = serialize_operation_request(&operation);

	if(send(instance->socket_reference, buffer, OPERATION_REQUEST_SIZE, 0) != OPERATION_REQUEST_SIZE){

		// Conection to instance fails. Must be removed and replanify all instances.
		log_warning(coordinador_log , "It was an error trying to send GET OPERATION to an Instance. Aborting execution");
		remove_client(server,instance->socket_id);

		// Verify free
		free(instance);
		return false;
	}else{

		response_status = receive_response_from_instance(instance);

		if(response_status)
		value_instance = receive_value_from_instance(instance , esi_request->payload_size);

	}
	free(buffer);

	// Must return value. Ignore in this case.
	return response_status;
}

bool send_set_operation(t_operation_request* esi_request, operation_type_e operation_type, t_connected_client *instance , char * payload_value){


	t_operation_request operation;
	strcpy(operation.key, esi_request->key);
	operation.operation_type = operation_type;
	operation.payload_size = esi_request->payload_size;

	bool response_status = false;

	log_info(coordinador_log , "Attemting to send SET OPERATION to Instance");

	void *buffer = serialize_operation_request(&operation);

	if(send(instance->socket_reference, buffer, OPERATION_REQUEST_SIZE, 0) != OPERATION_REQUEST_SIZE){

		// Conection to instance fails. Must be removed and replanify all instances.
		log_warning(coordinador_log , "It was an error trying to send SET OPERATION to an Instance. Aborting execution");
		remove_client(server,instance->socket_id);

		// Verify free
		free(instance);
		return false;
	}else{

		int payload_size = operation.payload_size;

		if( send(instance->socket_reference, payload_value, payload_size, 0) != payload_size){
			log_warning(coordinador_log , "It was an error trying to send value to an Instance. Aborting execution");
			remove_client(server,instance->socket_id);

			// Verify free
			free(instance);
			return false;
		}

		response_status = receive_response_from_instance(instance);

	}
	free(buffer);

	// Must return value. Ignore in this case.
	return response_status;
}

t_connected_client* select_intance_LSU(){
	// TODO
	return NULL;
}

t_connected_client* select_intance_EL(){
	// TODO
	return NULL ;
}

t_connected_client* select_intance_KE(){

	if(instancia_actual != list_size(connected_instances) && instancia_actual != 0){
		instancia_actual++;
	}else{
		instancia_actual=0;
	}

	return list_get(connected_instances, instancia_actual);
}

t_connected_client* select_instancia(){

	// MEJORAR COMO ESTA EN INSTANCIA
	switch(coordinador_setup.ALGORITMO_DISTRIBUCION){
	case LSU:
		return select_intance_LSU();
		break;
	case EL:
		return select_intance_EL();
		break;
	case KE:
		return select_intance_KE();
		break;
	}

	return NULL;

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

	char* buffer = malloc(OPERATION_RESPONSE_SIZE);

	if (recv(socket, buffer, OPERATION_RESPONSE_SIZE, MSG_WAITALL) < OPERATION_RESPONSE_SIZE) {
		log_warning(coordinador_log , "PLANNER has disconnected");
		remove_client(server,socket_id );
		free(buffer);
		return;
	}
}

void instance_disconected(int socket_id){

	int buffer , aux_size ;

	int valor = recv(socket, buffer, aux_size, MSG_WAITALL);

	if (valor == -1) {

		log_warning(coordinador_log , "INSTANCE has disconnected");
		remove_instance(server,socket_id );
	}

}

void on_server_read(tcp_server_t* server, int client_socket, int socket_id){

	// Verifico que proceso estoy leyendo:
	t_connected_client* client = find_connected_client(socket_id);

	if(client == NULL){
		// TODO: VER QUE HACEMOS! CLIENTE INVALIDO, no deberia pasar nunca
		return;
	}

	switch(client->instance_type){
	case ESI:
		handle_esi_read(client, client_socket);
		break;
	case REDIS_INSTANCE:
		instance_disconected(client->socket_id);
		// TODO : Verificar desconexion. Ojo , puede pasar que se desconecte en el
		//		  medio de una escucha de otro proceso. Se deberia contemplar.
		//		  Ademas las instancias me devuelven un valor en el GET , por lo que
		// 		  no podria hacerlo tan parecido a la desconexion del planificador.
		// 		  Se me ocurre un flag de utilizacion al esperar el SET.
		break;
	case PLANNER:
		planner_disconected(client->socket_id);
		break;
	case COORDINATOR:
		break;
	}

}

int serialize_data_instancia(void *object, int nBytes, void **buffer, int *lastIndex){
    void * auxiliar = NULL;
    auxiliar  = realloc(*buffer, nBytes+*lastIndex);
    if(auxiliar  == NULL) {
        return -1;
    }
    *buffer = auxiliar;
    if (memcpy((*buffer + *lastIndex), object, nBytes) == NULL) {
        return -2;
    }
    *lastIndex += nBytes;
    return 0;
}

void on_server_command(tcp_server_t* server){
	// TODO: FALTA HACER!
}


void destroy_connected_client(t_connected_client* connected_client){
	free(connected_client);
}



void server_planner_console_accept(tcp_server_t* server, int client_socket, int socket_id){

	void *header_buffer = malloc(CONNECTION_HEADER_SIZE);

	int res = recv(client_socket, header_buffer, CONNECTION_HEADER_SIZE, MSG_WAITALL);
	if (res <= 0) {
		log_error(coordinador_log, "Error receiving handshake request from PLANNER CONSOLE");
		tcpserver_remove_client(server, socket_id);
		free(header_buffer);
		return;
	}

	t_connection_header *connection_header = deserialize_connection_header(header_buffer);

	send_message_clients(connection_header, client_socket, socket_id);

}

void server_planner_console_read(tcp_server_t* server, int client_socket, int socket_id){

	// Handle PLANNER CONSOLE REQUEST

	// DEFINE PROTOCOL WITH PLANNER CONSOLE.

//	char* buffer = malloc(OPERATION_REQUEST_SIZE);
//
//	if (recv(socket, buffer, OPERATION_REQUEST_SIZE, MSG_WAITALL) < OPERATION_REQUEST_SIZE) {
//		log_warning(coordinador_log, "ESI Disconnected: %s", client->instance_name);
//		free(buffer);
//		remove_client(server, client->socket_id); //TODO: NO HACE FALTA EL FIND PORQUE YA LO TENGO. SE PUEDE MEJORAR
//		return;
//	}
//
//	t_operation_request* esi_request = deserialize_operation_request(buffer);
//
//	handle_esi_request(esi_request, client, socket);
//
//	free(esi_request);
//	free(buffer);
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

int main(void) {

	print_header();
	create_log();
	loadConfig();
	log_inicial_consola();

	pthread_mutex_init(&mutex_all, NULL);

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
