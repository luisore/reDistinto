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
		coordinador_setup.NOMBRE_INSTANCIA = string_duplicate(config_get_string_value(config,"NOMBRE_INSTANCIA"));
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
	if(connected_clients != NULL) list_destroy_and_destroy_elements(connected_clients, destroy_connected_client);
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

}


void create_tcp_server(){
	connected_clients = list_create();

	server = tcpserver_create(coordinador_setup.NOMBRE_INSTANCIA, coordinador_log,
			coordinador_setup.CANTIDAD_MAXIMA_CLIENTES,
			coordinador_setup.TAMANIO_COLA_CONEXIONES,
			coordinador_setup.PUERTO_ESCUCHA_CONEXIONES, true);

	if(server == NULL){
		log_error(coordinador_log, "Could not create TCP server. Aborting execution.");
		exit_program(EXIT_FAILURE);
	}
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

void mandarAlPlanificador(char * recurso, int client_socket, operation_type_e t)
{
	t_coordinator_operation_request e;
	strcpy(e.key, recurso);
	e.operation_type = t;

	void *buffer = serialize_coordinator_operation_request(&e);

	int r = send(client_socket, buffer, COORDINATOR_OPERATION_REQUEST_SIZE, 0);

	free(buffer);

	int bytesReceived = 0;
	void *res_buffer = malloc(COORDINATOR_OPERATION_REQUEST_SIZE);

	bytesReceived = recv(client_socket, res_buffer, OPERATION_RESPONSE_SIZE, MSG_WAITALL);

	if (bytesReceived < OPERATION_RESPONSE_SIZE) {
		log_error(coordinador_log, "Error!");

		log_error(coordinador_log, "Bytes leidos: %d | Esperados: %d",
				bytesReceived, OPERATION_RESPONSE_SIZE);

		free(res_buffer);
		return;
	}

	t_operation_response *response =
			deserialize_operation_response(res_buffer);

	log_info(coordinador_log, "Respuesta: %d", response->operation_result);
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

	if(connection_header->instance_type == PLANNER)
	{
		printf("Se conecto el planificador CHE\n");

		mandarAlPlanificador("materias:K3002", client_socket, GET);
		mandarAlPlanificador("LALALA1", client_socket, GET);
		mandarAlPlanificador("materias:K3002", client_socket, SET);
		mandarAlPlanificador("LALALA2", client_socket, SET);
		mandarAlPlanificador("materias:K3002", client_socket, STORE);
		mandarAlPlanificador("LALALA3", client_socket, STORE);
	}

	//TODO: Modularizar

	t_connected_client* connected_client = malloc(sizeof(t_connected_client));
	strcpy(&(connected_client->instance_name), connection_header->instance_name);
	connected_client->instance_type = connection_header->instance_type;
	connected_client->socket_id = socket_id;
	list_add(connected_clients, (void*)connected_client);

	free(header_buffer);
	free(connection_header);
	free(ack_buffer);
}

t_connected_client* find_connected_client(int socket_id){
	bool is_linked_to_socket(void* conn_client){
		t_connected_client* connected_client = (t_connected_client*)conn_client;
		return connected_client->socket_id == socket_id;
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
	switch(esi_request->operation_type){
	case GET:
		log_info(coordinador_log, "Handling GET from ESI: %s. Key: %s.", client->instance_name, esi_request->key);
		// TODO: HACER EL GET
		send_response_to_esi(socket, client, OP_SUCCESS);
		break;
	case STORE:
		log_info(coordinador_log, "Handling STORE from ESI: %s. Key: %s.", client->instance_name, esi_request->key);
		// TODO: HACER EL STORE
		send_response_to_esi(socket, client, OP_SUCCESS);
		break;
	case SET:
		// leer del buffer el contenido y procesar
		//TODO: HANDLE!
		break;
	}
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

void on_server_read(tcp_server_t* server, int client_socket, int socket_id){
    // Verifico que instancia estoy leyendo:
	t_connected_client* client = find_connected_client(socket_id);

	if(client == NULL){
		// TODO: VER QUE HACEMOS! CLIENTE INVALIDO, no deberia pasar nunca
		return;
	}

	switch(client->instance_type){
	case ESI:
		handle_esi_read(client, client_socket);
		break;
	}

	/*
	// 1. Verifica que lo que haya llegado este completo y de acuerdo al protocolo.
	void *package_buffer = malloc(CONNECTION_PACKAGE_SIZE);

	if (recv(client_socket, package_buffer, CONNECTION_PACKAGE_SIZE, MSG_WAITALL) < CONNECTION_PACKAGE_SIZE) {
		log_error(coordinador_log, "Error receiving status from ESI!");
		free(package_buffer);
		tcpserver_remove_client(server, socket_id);
		return;
	}

	t_response_process* abstract_response = deserialize_abstract_response(package_buffer);
	switch(abstract_response->instance_type){
		case ESI:
			log_info(coordinador_log, "EL proceso que envia informacion fue un ESI");
			enviar_respuesta_esi(client_socket, socket_id);
			break;
		case PLANNER:
			log_info(coordinador_log, "EL proceso que envia informacion fue el PLANIFICADOR");
			break;
		case REDIS_INSTANCE:
			log_info(coordinador_log, "EL proceso que envia informacion fue una INSTANCIA");

			//void* estado=tratar_instancia(abstract_response);
			//Agregar a lista de instancias en estado status (inicializacion de instancia)
			//enviar_respuesta_instancia(client_socket, socket_id, estado);
			break;
	}

	free(package_buffer);
	free(abstract_response);
*/


}


/*
void* tratar_instancia(t_response_process* abstract_response){
	void* buffer = malloc(4);
	int lastIndex = 0;

	serialize_data_instancia(&(abstract_response->response),4, &buffer, &lastIndex);

	return buffer;
}*/

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


void enviar_respuesta_instancia(int instancia_socket, int socket_id, void *estado){
	/*switch(estado){
	case STATUS:
		lista_instancias = list_create();
		t_coordinador_request_instancia coordinador_request;
		t_instancia instancia;
		instancia->id_socket=socket_id;
		instancia->tamanio_entrada_bytes=coordinador_request->TAMANIO_ENTRADA_BYTES;
		instancia->vantidad_entradas=coordinador_request->CANTIDAD_ENTRADAS;
		list_add(lista_instancias, instancia);
		strcpy(coordinador_request.coordinador_name, coordinador_setup.NOMBRE_INSTANCIA);
		strcpy(coordinador_request.TAMANIO_ENTRADA_BYTES, coordinador_setup.TAMANIO_ENTRADA_BYTES);
		strcpy(coordinador_request.CANTIDAD_ENTRADAS, coordinador_setup.CANTIDAD_ENTRADAS);
		void *buffer = serialize_coordinador_request_instancia(&coordinador_request);
		int result = send(instancia_socket, buffer, PLANNER_REQUEST_SIZE, 0);
		if (result <= 0) {
			log_error(coordinador_log, "Signal execute next to ESI failed for ID: %d");
			tcpserver_remove_client(server, socket_id);
		}
		free(buffer);
		break;
	}

	free(estado); */
}

void destroy_connected_client(t_connected_client* connected_client){
	free(connected_client);
}



int main(void) {

	print_header();
	create_log();
	loadConfig();
	log_inicial_consola();


	create_tcp_server();
	tcpserver_run(server, before_tpc_server_cycle, on_server_accept, on_server_read, on_server_command);



	print_goodbye();
	exit_program(EXIT_SUCCESS);

	return 0;

}
