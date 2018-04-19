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
	coordinator_port = string_duplicate(config_get_string_value(config, "coordinatorPort"));
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

	log_info(console_log, "Loaded configuration. Coordinator IP: %s PORT: %s. Initial estimation: %d, Planification Algorithm: %s, Listen on port: %d",
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
	server = tcpserver_create(instance_name, "planner_server.log", max_clients, connection_queue_size, port_to_listen, true);
	if(server == NULL){
		log_error(console_log, "Could not create TCP server. Aborting execution.");
		exit_gracefully(EXIT_FAILURE);
	}
}

int connect_to_server(char *ip, char *port) {

	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;    // Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;  // Indica que usaremos el protocolo TCP

	getaddrinfo(ip, port, &hints, &server_info);  // Carga en server_info los datos de la conexion

	int server_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	int res = connect(server_socket, server_info->ai_addr, server_info->ai_addrlen);

	if (res < 0) {
		if(server_socket != 0) close(server_socket);
		exit_gracefully(EXIT_FAILURE);
	}

	freeaddrinfo(server_info);  // No lo necesitamos mas

	log_info(console_log, "Connected to server IP: %s PORT: %s");
	return server_socket;
}

void perform_connection_handshake(int server_socket){
	t_connection_header connection_header;
	strcpy(connection_header.instance_name, instance_name);
	connection_header.instance_type = ESI;

	log_trace(console_log, "Sending handshake message...");
	int result = send(server_socket, &connection_header, sizeof(t_connection_header), 0);

	if (result <= 0) {
		log_error(console_log, "Could not perform handshake with server. Send message failed");
		exit_gracefully(EXIT_FAILURE);
	}
	log_trace(console_log, "Handshake message sent. Waiting for response...");

	t_ack_message * ack_message = malloc(sizeof(t_ack_message));

	if (recv(socket, ack_message, sizeof(t_ack_message), 0) <= 0) {
		log_error(console_log, "Error receiving handshake response. Aborting execution.");
		exit_gracefully(EXIT_FAILURE);
	}

	log_info(console_log, "Handshake successful with server: %s.", ack_message->instance_name);

	free(ack_message);
}

void connect_with_coordinator() {
	log_info(console_log, "Connecting to Coordinator.");
	coordinator_socket = connect_to_server(coordinator_ip, coordinator_port);

	perform_connection_handshake(coordinator_socket);
	log_info(console_log, "Successfully connected to Coordinator.");
}

void send_execute_next_to_esi(int esi_socket){
	t_planner_request planner_request;
	strcpy(planner_request.planner_name, instance_name);

	int result = send(esi_socket, &planner_request, sizeof(t_planner_request), 0);

	if (result <= 0) {
		log_error(console_log, "Signal execute next to ESI failed for ID: %d");
		// TODO: close connection with ESI
	}
}

void before_tpc_server_cycle(tcp_server_t* server){
	// ACÁ DEBERÍA IR LA LÓGICA DE SCHEDULING
}


void on_server_accept(tcp_server_t* server, int client_socket){
	t_connection_header *connection_header = malloc(sizeof(t_connection_header));

	if (recv(socket, connection_header, sizeof(t_connection_header), 0) <= 0) {
		log_error(console_log, "Error receiving handshake request from TCP Client!");
	}
	log_info(console_log, "Received handshake from TCP Client: %s", connection_header->instance_name);
	free(connection_header);

	t_ack_message ack_message;
	strcpy(ack_message.instance_name, instance_name);

	if( send(client_socket, &ack_message, sizeof(t_ack_message), 0) != sizeof(t_ack_message))
	{
		log_error(console_log, "Could not send handshake acknowledge to TCP client.");
	} else {
		log_info(console_log, "Successfully connected to TCP Client: %s", connection_header->instance_name);
	}

	// ESTO NO DEBERÍA IR ACÁ. ES PARA LA PRUEBA DE CONCEPTO
	// Envía al ESI que ejecute la primera instrucción
	// Esto lo tendría que manejar el algoritmo de scheduling para determinar a quién le toca.
	send_execute_next_to_esi(client_socket);
}

void on_server_read(tcp_server_t* server, int client_socket, int socket_id){
	// ESTO PODRÍA NO HACER NADA ACÁ, Y MANEJAR LOS READs SINCRÓNICOS EN EL SCHEDULER...

	t_esi_status_response *esi_status_response = malloc(sizeof(t_esi_status_response));

	if (recv(socket, esi_status_response, sizeof(t_esi_status_response), 0) <= 0) {
		log_error(console_log, "Error receiving status from ESI!");
	}
	log_info(console_log, "Received Status from ESI: %s", esi_status_response->status);

	switch(esi_status_response->status){
		case ESI_IDLE:
			// Por ahora, mando la siguiente operacion
			log_info(console_log, "ESI is IDLE. Signal next operation");
			send_execute_next_to_esi(client_socket);
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
	if(coordinator_port != NULL) free(coordinator_port);
	if(initial_blocked_keys != NULL) free(initial_blocked_keys);

	if(coordinator_socket != 0) close(coordinator_socket);

	if(server != NULL) tcpserver_destroy(server);

	exit(retVal);
}
