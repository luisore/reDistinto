#include "Planificador.h"

int main(void) {

	print_header();

	inicializarListasEsi();

	if (inicializar() < 0) {
		liberarRecursos(EXIT_FAILURE);
		return -1;
	}

	pthread_mutex_init(&mutexConsola, NULL);
	pthread_create(&hiloConsola, NULL, (void*) escucharConsola, NULL);

	pthread_mutex_init(&mutexPrincipal, NULL);
	pthread_create(&hiloPrincipal, NULL, (void*) iniciarPlanificador, NULL);

	pthread_join(hiloConsola, NULL);
	pthread_join(hiloPrincipal, NULL);

	liberarRecursos(EXIT_SUCCESS);
	return 0;
}

void print_header() {
	printf("\n\t\e[31;1m=========================================\e[0m\n");
	printf("\t.:: Bievenido a ReDistinto ::.");
	printf("\n\t\e[31;1m=========================================\e[0m\n\n");
}

int inicializar() {

	create_log();

	if (cargarConfiguracion(console_log, PLANNER_CFG_FILE) < 0) {
		log_error(console_log, "No se encontró el archivo de configuración");
		return -1;
	}

	return 0;
}

void escucharConsola() {
	log_error(console_log, "Se inicio hilo con la consola");

	while (true) {
		if (consolaLeerComando(console_log) == TERMINAR_CONSOLA) {
			pthread_exit(0);
			return;
		}
	}
}

void iniciarPlanificador() {

	//connect_with_coordinator();

	//create_tcp_server();

	//	tcpserver_run(server, before_tpc_server_cycle, on_server_accept,
	//			on_server_read, on_server_command);

	pthread_exit(0);
}

void create_log() {
	console_log = log_create("planificador.log", "ReDistinto-Planificador",
	false, LOG_LEVEL_ERROR);

	if (console_log == NULL) {
		printf("Could not create log. Execution aborted.");
		exit_gracefully(EXIT_FAILURE);
	}
}

void create_tcp_server() {
	server = tcpserver_create(planificador_setup.NOMBRE_INSTANCIA, console_log,
			planificador_setup.CANTIDAD_MAXIMA_CLIENTES,
			planificador_setup.TAMANIO_COLA_CONEXIONES,
			planificador_setup.PUERTO_ESCUCHA_CONEXIONES, true);
	if (server == NULL) {
		log_error(console_log,
				"Could not create TCP server. Aborting execution.");
		exit_gracefully(EXIT_FAILURE);
	}
}

void connect_with_coordinator() {
	log_info(console_log, "Connecting to Coordinator.");
	coordinator_socket = connect_to_server(planificador_setup.IP_COORDINADOR,
			planificador_setup.PUERTO_COORDINADOR, console_log);
	if (coordinator_socket <= 0) {
		exit_gracefully(EXIT_FAILURE);
	}

	if (!perform_connection_handshake(coordinator_socket,
			planificador_setup.NOMBRE_INSTANCIA, PLANNER, console_log)) {
		exit_gracefully(EXIT_FAILURE);
	}
	log_info(console_log, "Successfully connected to Coordinator.");
}

void send_execute_next_to_esi(int esi_socket, int socket_id) {
	t_planner_request planner_request;
	strcpy(planner_request.planner_name, planificador_setup.NOMBRE_INSTANCIA);

	void *buffer = serialize_planner_request(&planner_request);

	int result = send(esi_socket, buffer, PLANNER_REQUEST_SIZE, 0);

	if (result <= 0) {
		log_error(console_log, "Signal execute next to ESI failed for ID: %d");
		tcpserver_remove_client(server, socket_id);
	}
	free(buffer);
}

void before_tpc_server_cycle(tcp_server_t* server) {
	// ACÁ DEBERÍA IR LA LÓGICA DE SCHEDULING
}

/**
 * Funcion que se ejecuta cuando un externo se conecta a nuestro servidor
 */
void on_server_accept(tcp_server_t* server, int client_socket, int socket_id) {
	void *header_buffer = malloc(CONNECTION_HEADER_SIZE);

	int res = recv(client_socket, header_buffer, CONNECTION_HEADER_SIZE,
	MSG_WAITALL);
	if (res <= 0) {
		log_error(console_log,
				"Error receiving handshake request from TCP Client!");
		tcpserver_remove_client(server, socket_id);
		free(header_buffer);
		return;
	}

	t_connection_header *connection_header = deserialize_connection_header(
			header_buffer);

	log_info(console_log, "Received handshake from TCP Client: %s",
			connection_header->instance_name);

	if (connection_header->instance_type == ESI) {
		ESI_STRUCT * esi = nuevoESI(0, client_socket, socket_id);
		agregarNuevoEsi(esi);
	}

	free(header_buffer);
	free(connection_header);

	t_ack_message ack_message;

	strcpy(ack_message.instance_name, planificador_setup.NOMBRE_INSTANCIA);

	void *ack_buffer = serialize_ack_message(&ack_message);

	if (send(client_socket, ack_buffer, ACK_MESSAGE_SIZE, 0)
			!= ACK_MESSAGE_SIZE) {
		log_error(console_log,
				"Could not send handshake acknowledge to TCP client.");
		tcpserver_remove_client(server, socket_id);
	} else {
		log_info(console_log, "Successfully connected to TCP Client: %s",
				connection_header->instance_name);
	}

	free(ack_buffer);
}

void on_server_read(tcp_server_t* server, int client_socket, int socket_id) {
	void *res_buffer = malloc(ESI_STATUS_RESPONSE_SIZE);

	if (recv(client_socket, res_buffer, ESI_STATUS_RESPONSE_SIZE, MSG_WAITALL)
			< ESI_STATUS_RESPONSE_SIZE) {
		log_error(console_log, "Error receiving status from ESI!");
		free(res_buffer);
		tcpserver_remove_client(server, socket_id);
		return;
	}

	t_esi_status_response *esi_status_response =
			deserialize_esi_status_response(res_buffer);
	log_info(console_log, "Received Status from ESI: %d",
			esi_status_response->status);

	switch (esi_status_response->status) {
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

void on_server_command(tcp_server_t* server) {
	// TODO: FALTA HACER!
	int valread;
	char buffer[1024];

	valread = read(STDIN_FILENO, buffer, 1024);

	// To skip the \n...
	buffer[valread - 1] = '\0';

	if (strcmp("exit", buffer) == 0) {
		printf("Exit command received.\n");
		log_info(server->logger, "TCP Server %s. Exit requested by console.",
				server->name);
		exit_gracefully(EXIT_SUCCESS);
	} else {
		printf("Unknown command: %s. Enter 'exit' to exit.\n", buffer);
	}
}

void liberarRecursos(int tipoSalida) {
	log_destroy(console_log);

	int i = 0;
	while (planificador_setup.CLAVES_INICIALMENTE_BLOQUEADAS[i] != NULL) {
		free(planificador_setup.CLAVES_INICIALMENTE_BLOQUEADAS[i]);
		i++;
	}
	free(planificador_setup.CLAVES_INICIALMENTE_BLOQUEADAS);

	pthread_mutex_destroy(&mutexConsola);
	pthread_mutex_destroy(&mutexPrincipal);

	liberarRecursosEsi();

	exit_gracefully(tipoSalida);
}

/**
 * Aplica el algoritmo de planificacion
 */
void applyPlaningAlgorithm() {
	//TODO: implementacion pendiente
}

/**
 * Retorna el siguiente ESI a ejecutar segun el algoritmo
 */
void getNextESI() {
	//TODO: implementacion pendiente
}

/**
 * Desaloja el ESI actual
 */
void moveOutCurrentESI() {
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
bool isResourceAvailable() {
	//TODO: implementacion pendiente
	return true;
}

/**
 * Bloquea un recurso
 */
// TODO: falta pasar el recurso por parametro
void lockResource() {

}

/**
 * Desbloquea un recurso
 */
// TODO: falta pasar el recurso por parametro
void unlockResource() {

}

/**
 * Notifica al Coordinador del resultado de bloquear un recurso
 */
void sendLockResourceOperationResult(bool p_result) {
	//TODO: implementacion pendiente
}

/**
 * Notifica al Coordinador del resultado de liberar un recurso
 */
void sendUnlockResourceOperationResult(bool p_result) {
	//TODO: implementacion pendiente
}

void exit_gracefully(int retVal) {

	if (coordinator_socket != 0)
		close(coordinator_socket);

	if (server != NULL)
		tcpserver_destroy(server);

	exit(retVal);
}
