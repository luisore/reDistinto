#include "Planificador.h"

int main(void) {

	print_header();

	inicializarListasEsi();

	if (inicializar() < 0) {
		liberarRecursos(EXIT_FAILURE);
		return -1;
	}

	pthread_mutex_init(&mutexConsola, NULL);
	pthread_mutex_init(&mutexPrincipal, NULL);
	pthread_mutex_init(&mutexPlanificacion, NULL);

	pthread_create(&hiloConsola, NULL, (void*) escucharConsola, NULL);
	pthread_create(&hiloPrincipal, NULL, (void*) iniciarPlanificador, NULL);
	pthread_create(&hiloPlanificacion, NULL, (void*) ejecutarPlanificacion, NULL);

	pthread_join(hiloConsola, NULL);
	pthread_join(hiloPrincipal, NULL);
	pthread_join(hiloPlanificacion, NULL);

	liberarRecursos(EXIT_SUCCESS);
	return 0;
}

void print_header() {
	printf("\n\t\e[31;1m=========================================\e[0m\n");
	printf("\t.:: Bievenido a ReDistinto ::.");
	printf("\n\t\e[31;1m=========================================\e[0m\n\n");
}

int inicializar() {

	if (create_log() == EXIT_FAILURE)
		exit_gracefully(EXIT_FAILURE);

	if (cargarConfiguracion(console_log, PLANNER_CFG_FILE) < 0) {
		log_error(console_log, "No se encontró el archivo de configuración");
		return -1;
	}

	float alpha = (float) planificador_setup.ALPHA / 100;
	setAlpha(alpha);

	setEstimacionInicial(planificador_setup.ESTIMACION_INICIAL);

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
	//conectarseConCoordinador();

	create_tcp_server();

	tcpserver_run(server, before_tpc_server_cycle, on_server_accept,
			on_server_read, on_server_command);

	pthread_exit(0);
}

void ejecutarPlanificacion() {
	while (true) {
		pthread_mutex_lock(&mutexPlanificacion);

		// Hay algun esi conectado?
		if (cantidadEsiTotales() == 0) {
			log_info(console_log, "\n\n **************** NO HAY ESI *******************\n");

			// Espero a que se conecte algun esi, para que esto no itere infinitamente
			pthread_mutex_lock(&mutexPlanificacion);
		}

		log_info(console_log, "\n\n **************** HAY ESI *******************\n");

		aplicar_algoritmo_planificacion();

		if (esiEjecutando != NULL) {
			log_info(console_log, "ESI actual\tid: %d \tTiempo estimado: %d\n",
					esiEjecutando->id, esiEjecutando->tiempoEstimado);

			ejecutarSiguienteESI(esiEjecutando->client_socket, esiEjecutando->socket_id);


			// TODO: ACA VA LO DEL COORDINADOR
			escucharCoordinador();


			// Aumento contadores esi actual
			esiEjecutando->tiempoRafagaActual++;
			esiEjecutando->tiempoEstimado--;
			if(esiEjecutando->tiempoEstimado < 0)
				esiEjecutando->tiempoEstimado = 0;


			// ACA LE TENGO QUE ESPERAR AL ESTADO DEL ESI
			int estado = esperarEstadoDelEsi(esiEjecutando->client_socket,
					esiEjecutando->socket_id);

			switch (estado) {
			case -1:
				log_info(console_log, "Error al pedir el estado del ESI. Abortando ESI");
				terminarEsiActual();
				break;
			case ESI_IDLE:
				log_info(console_log, "El ESI puede seguir ejecutando");
				break;
			case ESI_BLOCKED:
				log_info(console_log, "El ESI esta bloqueado");
				// El bloqueo lo manejo por la parte de recursos
				break;
			case ESI_FINISHED:
				log_info(console_log, "El ESI termino");
				tcpserver_remove_client(server, esiEjecutando->socket_id);
				terminarEsiActual();
				break;
			}
		}

		// Realizar incrementos de contadores
		nuevoCicloDeCPU();

		pthread_mutex_unlock(&mutexPlanificacion);
	}
	pthread_exit(0);
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

void conectarseConCoordinador() {
	log_info(console_log, "Conectando al Coordinador ...");
	coordinator_socket = connect_to_server(planificador_setup.IP_COORDINADOR,
			planificador_setup.PUERTO_COORDINADOR, console_log);
	if (coordinator_socket <= 0) {
		exit_gracefully(EXIT_FAILURE);
	}

	if (!perform_connection_handshake(coordinator_socket,
			planificador_setup.NOMBRE_INSTANCIA, PLANNER, console_log)) {
		exit_gracefully(EXIT_FAILURE);
	}
	log_info(console_log, "Conexion exitosa al Coordinador.");
}

void ejecutarSiguienteESI(int esi_socket, int socket_id) {
	t_planner_execute_request planner_request;
	//strcpy(planner_request.planner_name, planificador_setup.NOMBRE_INSTANCIA);
	char* code = "0";
	strcpy(planner_request.planner_name, code);

	void *buffer = serialize_planner_execute_request(&planner_request);

	int result = send(esi_socket, buffer, PLANNER_REQUEST_SIZE, 0);

	if (result <= 0) {
		log_error(console_log, "Signal execute next to ESI failed for ID: %d");
		tcpserver_remove_client(server, socket_id);
	}
	free(buffer);
}

int esperarEstadoDelEsi(int esi_socket, int socket_id) {
	int esi_status = -1, bytesReceived = 0;
	void *res_buffer = malloc(ESI_STATUS_RESPONSE_SIZE);

	bytesReceived = recv(esi_socket, res_buffer, ESI_STATUS_RESPONSE_SIZE,
			MSG_WAITALL);

	if (bytesReceived < ESI_STATUS_RESPONSE_SIZE) {
		log_error(console_log, "Error receiving status from ESI!");

		log_error(console_log, "Bytes leidos: %d | Esperados: %d",
				bytesReceived, ESI_STATUS_RESPONSE_SIZE);

		free(res_buffer);
		tcpserver_remove_client(server, socket_id);
		return esi_status;
	}

	t_esi_status_response *esi_status_response =
			deserialize_esi_status_response(res_buffer);

	log_info(console_log, "Estado del ESI: %d", esi_status_response->status);

	esi_status = esi_status_response->status;

	free(res_buffer);
	free(esi_status_response);
	return esi_status;
}

void before_tpc_server_cycle(tcp_server_t* server) {
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

	if (connection_header->instance_type == ESI) {
		log_info(console_log, "************* NUEVO ESI");

		int id_esi = generarId();
		ESI_STRUCT * esi = nuevoESI(id_esi, client_socket, socket_id);
		agregarNuevoEsi(esi);

		// Nuevo esi -> libero el hilo de planificacion
		pthread_mutex_unlock(&mutexPlanificacion);
	}

	free(header_buffer);
	free(connection_header);
	free(ack_buffer);
}

void on_server_read(tcp_server_t* server, int client_socket, int socket_id) {
}

void on_server_command(tcp_server_t* server) {
}

void escucharCoordinador(){
	int bytesReceived = 0;
	void *res_buffer = malloc(ESI_STATUS_RESPONSE_SIZE);

	bytesReceived = recv(coordinator_socket, res_buffer, COORDINATOR_OPERATION_REQUEST_SIZE,
	MSG_WAITALL);

	if (bytesReceived < COORDINATOR_OPERATION_REQUEST_SIZE) {
		log_error(console_log, "Error!");

		log_error(console_log, "Bytes leidos: %d | Esperados: %d",
				bytesReceived, COORDINATOR_OPERATION_REQUEST_SIZE);

		free(res_buffer);
		return;
	}

	t_coordinator_operation_request *request =
			deserialize_coordinator_operation_request(res_buffer);

	log_info(console_log, "El coordinador solicita: %d", request->operation_type);

	char * key = &request->key[0];

	int result;
	switch (request->operation_type) {
		case GET: {
			// 1 - Me fijo si el recurso esta bloqueado por otro esi
			result = estaBloqueadoPor(esiEjecutando, key);

			if(result == 0)
			{
				// Esta bloqueado por otro esi
				responderCoordinador(coordinator_socket, OP_BLOCKED);
			}
			else if(result == 1) {
				// Ya esta bloqueado por el esi actual
				responderCoordinador(coordinator_socket, OP_ERROR);
			}
			else {
				// No esta bloqueado
				bloquearRecurso(key);
				responderCoordinador(coordinator_socket, OP_SUCCESS);
			}

			break;
		}
		case SET:{
			// 1 - Me fijo si el recurso esta bloqueado por otro esi
			result = estaBloqueadoPor(esiEjecutando, key);

			if(result == 0)
			{
				// Esta bloqueado por otro esi
				responderCoordinador(coordinator_socket, OP_BLOCKED);
			}
			else if(result == 1) {
				// Ya esta bloqueado por el esi actual
				responderCoordinador(coordinator_socket, OP_SUCCESS);
			}
			else {
				responderCoordinador(coordinator_socket, OP_ERROR);
			}
			break;
		}
		case STORE: {
			// 1 - Me fijo si el recurso esta bloqueado por otro esi
			result = estaBloqueadoPor(esiEjecutando, key);

			if(result == 0)
			{
				// Esta bloqueado por otro esi
				responderCoordinador(coordinator_socket, OP_BLOCKED);
			}
			else if(result == 1) {
				// Ya esta bloqueado por el esi actual
				liberarRecurso(key);
				responderCoordinador(coordinator_socket, OP_SUCCESS);
			}
			else {
				// No esta bloqueado
				responderCoordinador(coordinator_socket, OP_ERROR);
			}
			break;
		}
		default:
			break;
	}

	free(res_buffer);
	free(request);
}

int generarId() {
	return esi_id++;
}

void aplicar_algoritmo_planificacion() {
	switch (planificador_setup.ALGORITMO_PLANIFICACION) {
	case SJF_SD:
		aplicarSJF(false);
		break;
	case SJF_CD:
		aplicarSJF(true);
		break;
	case HRRN:
		aplicarHRRN();
		break;
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
	pthread_mutex_destroy(&mutexPlanificacion);

	liberarRecursosEsi();
	liberarRecursosConfiguracion();

	exit_gracefully(tipoSalida);
}

void exit_gracefully(int retVal) {

	if (coordinator_socket != 0)
		close(coordinator_socket);

	if (server != NULL)
		tcpserver_destroy(server);

	exit(retVal);
}



void responderCoordinador(int socket, operation_result_e result){
	t_operation_response response;
	response.operation_result = result;

	void *buffer = serialize_operation_response(&response);

	int r = send(socket, buffer, OPERATION_RESPONSE_SIZE, 0);

	if (r <= 0) {
		log_error(console_log, "No se pudo enviar la respuesta al coordinador");
	}
	free(buffer);
}
