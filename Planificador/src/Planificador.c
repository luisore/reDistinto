#include "Planificador.h"

#include <stdbool.h>
#include <commons/string.h>
#include <sys/socket.h> // Para crear sockets, enviar, recibir, etc
#include <netdb.h> // Para getaddrinfo
#include <unistd.h> // Para close
#include "libs/protocols.h"


void load_configuration() {
	log_trace(console_log, "Loading configuration from file: %s", PLANNER_CFG_FILE);

	t_config *config = config_create(PLANNER_CFG_FILE);
	if(config == NULL){
		log_error(console_log, "Could not load configuration file.");
		exit_gracefully(EXIT_FAILURE);
	}

	instance_name = string_duplicate(config_get_string_value(config, "instanceName"));
	port_to_listen = config_get_int_value(config, "portToListen");
	coordinator_ip = string_duplicate(config_get_string_value(config, "coordinatorIP"));
	coordinator_port = config_get_int_value(config, "coordinatorPort");
	initial_estimation = config_get_int_value(config, "initialEstimation");
	planification_algorithm = config_get_int_value(config, "planificationAlgorithm");
	initial_blocked_keys = config_get_array_value(config, "initialBlockedKeys");
	max_clients = config_get_int_value(config, "maxClients");
	connection_queue_size = config_get_int_value(config, "connectionQueueSize");


	char* str_plan_algo;
	switch (planification_algorithm) {
		case SJF_CD:
			str_plan_algo = "SJF_CD";
			break;
		case SJF_SD:
			str_plan_algo = "SJF_SD";
			break;
		case HRRN:
			str_plan_algo = "HRRN";
			break;
		default:
			log_error(console_log, "Unknown planification algorithm from configuration. Abortin execution");
			config_destroy(config);
			exit_gracefully(EXIT_FAILURE);
			break;
	}

	log_info(console_log, "Loaded configuration. Coordinator IP: %s PORT: %d. Initial estimation: %d, Planification Algorithm: %s, Listen on port: %d",
			coordinator_ip, coordinator_port, initial_estimation, str_plan_algo, port_to_listen);


	int i = 0;
	while (initial_blocked_keys[i] != NULL) {
		log_info(console_log, "\tClave inicial bloqueada nro %d: %s", i + 1,
				initial_blocked_keys[i]);
		i++;
	}

	config_destroy(config);
}

void print_header(){
	printf("\n\t\e[31;1m=========================================\e[0m\n");
	printf("\t.:: Bievenido a ReDistinto ::.");
	printf("\n\t\e[31;1m=========================================\e[0m\n\n");
}

void create_log(){
	console_log = log_create("planificador.log", "ReDistinto-Planificador", false, LOG_LEVEL_TRACE);

	if(console_log == NULL){
		printf("Could not create log. Execution aborted.");
		exit_gracefully(EXIT_FAILURE);
	}
}

void create_tcp_server(){
	server = tcpserver_create(instance_name, console_log, max_clients, connection_queue_size, port_to_listen, true);
	if(server == NULL){
		log_error(console_log, "Could not create TCP server. Aborting execution.");
		exit_gracefully(EXIT_FAILURE);
	}
}

void connect_with_coordinator() {
	log_info(console_log, "Connecting to Coordinator.");
	coordinator_socket = connect_to_server(coordinator_ip, coordinator_port, console_log);
	if(coordinator_socket <= 0){
		exit_gracefully(EXIT_FAILURE);
	}

	if(!perform_connection_handshake(coordinator_socket, instance_name, PLANNER, console_log)){
		exit_gracefully(EXIT_FAILURE);
	}
	log_info(console_log, "Successfully connected to Coordinator.");
}

void send_execute_next_to_esi(int esi_socket, int socket_id){
	t_planner_request planner_request;
	strcpy(planner_request.planner_name, instance_name);

	void *buffer = serialize_planner_request(&planner_request);

	int result = send(esi_socket, buffer, PLANNER_REQUEST_SIZE, 0);

	if (result <= 0) {
		log_error(console_log, "Signal execute next to ESI failed for ID: %d");
		tcpserver_remove_client(server, socket_id);
	}
	free(buffer);
}

void before_tpc_server_cycle(tcp_server_t* server){
	// ACÁ DEBERÍA IR LA LÓGICA DE SCHEDULING
}


void on_server_accept(tcp_server_t* server, int client_socket, int socket_id){
	void *header_buffer = malloc(CONNECTION_HEADER_SIZE);

	int res = recv(client_socket, header_buffer, CONNECTION_HEADER_SIZE, MSG_WAITALL);
	if (res <= 0) {
		log_error(console_log, "Error receiving handshake request from TCP Client!");
		tcpserver_remove_client(server, socket_id);
		free(header_buffer);
		return;
	}

	t_connection_header *connection_header = deserialize_connection_header(header_buffer);
	log_info(console_log, "Received handshake from TCP Client: %s", connection_header->instance_name);
	free(header_buffer);
	free(connection_header);

	t_ack_message ack_message;
	strcpy(ack_message.instance_name, instance_name);
	void *ack_buffer = serialize_ack_message(&ack_message);

	if( send(client_socket, ack_buffer, ACK_MESSAGE_SIZE, 0) != ACK_MESSAGE_SIZE)
	{
		log_error(console_log, "Could not send handshake acknowledge to TCP client.");
		tcpserver_remove_client(server, socket_id);
	} else {
		log_info(console_log, "Successfully connected to TCP Client: %s", connection_header->instance_name);
	}

	free(ack_buffer);
}

void on_server_read(tcp_server_t* server, int client_socket, int socket_id){
	void *res_buffer = malloc(ESI_STATUS_RESPONSE_SIZE);

	if (recv(client_socket, res_buffer, ESI_STATUS_RESPONSE_SIZE, MSG_WAITALL) < ESI_STATUS_RESPONSE_SIZE) {
		log_error(console_log, "Error receiving status from ESI!");
		free(res_buffer);
		tcpserver_remove_client(server, socket_id);
		return;
	}

	t_esi_status_response *esi_status_response = deserialize_esi_status_response(res_buffer);
	log_info(console_log, "Received Status from ESI: %d", esi_status_response->status);

	switch(esi_status_response->status){
		case ESI_IDLE:
			// Por ahora, mando la siguiente operacion
			log_info(console_log, "ESI is IDLE. Signal next operation");
			send_execute_next_to_esi(client_socket, socket_id);
			break;
		case ESI_BLOCKED:
			// Por ahora, no hago nada...
			log_info(console_log, "ESI is BLOCKED.");
			break;
		case ESI_FINISHED:
			log_info(console_log, "ESI Finished execution");
			tcpserver_remove_client(server, socket_id);
			break;
	}

	free(res_buffer);
	free(esi_status_response);
}

void on_server_command(tcp_server_t* server){
	// TODO: FALTA HACER!
	int valread;
	char buffer[1024];

	valread = read(STDIN_FILENO, buffer, 1024);

	// To skip the \n...
	buffer[valread-1] = '\0';

	if(strcmp("exit", buffer) == 0){
		printf("Exit command received.\n");
		log_info(server->logger, "TCP Server %s. Exit requested by console.", server->name);
		exit_gracefully(EXIT_SUCCESS);
	} else {
		printf("Unknown command: %s. Enter 'exit' to exit.\n", buffer);
	}
}

int main(void) {
	print_header();

	create_log();

	load_configuration();

	//connect_with_coordinator();

	create_tcp_server();

	tcpserver_run(server, before_tpc_server_cycle, on_server_accept, on_server_read, on_server_command);

	return 0;
}

/**
 * Aplica el algoritmo de planificacion
 */
void applyPlaningAlgorithm(){
	//TODO: implementacion pendiente
}

/**
 * Retorna el siguiente ESI a ejecutar segun el algoritmo
 */
void getNextESI(){
	//TODO: implementacion pendiente
}

/**
 * Desaloja el ESI actual
 */
void moveOutCurrentESI(){
	//TODO: implementacion pendiente
}

/**
 * Le dice al ESI que puede seguir ejecutandose
 */
void continueExecutingESI() {
	//TODO: implementacion pendiente
}

/**
 * Verifica la disponibilidad de un recurso
 */
// TODO: falta pasar el recurso como parametro
bool isResourceAvailable(){
	//TODO: implementacion pendiente
	return true;
}

/**
 * Bloquea un recurso
 */
// TODO: falta pasar el recurso por parametro
void lockResource(){

}

/**
 * Desbloquea un recurso
 */
// TODO: falta pasar el recurso por parametro
void unlockResource(){

}


/**
 * Encola un ESI en la lista de bloqueados
 */
// TODO: falta pasar el ESI por parametro
void lockESI(){
	//TODO: implementacion pendiente
}

/**
 * Desencola un ESI de la lista de bloqueados
 */
// TODO: falta pasar el ESI por parametro
void unlockESI(){
	//TODO: implementacion pendiente
}

/**
 * Finaliza un ESI
 */
// TODO: falta pasar el ESI por parametro
void finishESI(){
	//TODO: implementacion pendiente
}

/**
 * Notifica al Coordinador del resultado de bloquear un recurso
 */
void sendLockResourceOperationResult(bool p_result){
	//TODO: implementacion pendiente
}

/**
 * Notifica al Coordinador del resultado de liberar un recurso
 */
void sendUnlockResourceOperationResult(bool p_result){
	//TODO: implementacion pendiente
}

void exit_gracefully(int retVal){
	if(instance_name != NULL) free(instance_name);
	if(coordinator_ip != NULL) free(coordinator_ip);
	if(initial_blocked_keys != NULL) free(initial_blocked_keys);

	if(coordinator_socket != 0) close(coordinator_socket);

	if(server != NULL) tcpserver_destroy(server);

	exit(retVal);
}
