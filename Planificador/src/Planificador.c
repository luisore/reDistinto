#include "Planificador.h"

void signal_catch(int signal){
	if (signal == SIGINT || signal == SIGKILL || signal == SIGSTOP || signal == SIGTSTP)
	{
		printf("\n¡ADIOS!\n");

		pthread_cancel(hiloPrincipal);
		pthread_cancel(hiloPlanificacion);
		pthread_cancel(hiloConsola);

		liberarRecursos(EXIT_SUCCESS);
	}
}

int main(void) {

	print_header();

	inicializarListasEsi();

	if (inicializar() < 0) {
		liberarRecursos(EXIT_FAILURE);
		return -1;
	}

	sem_init(&sem_esis, 0, 0);

	if (signal(SIGINT, signal_catch) == SIG_ERR)
		printf("\ncan't catch SIGINT\n");

	if (signal(SIGTSTP, signal_catch) == SIG_ERR)
			printf("\ncan't catch SIGINT\n");

	pthread_mutex_init(&mutexConsola, NULL);
	pthread_mutex_init(&mutexLog, NULL);
	pthread_mutex_init(&mutexPlanificador, NULL);

	pthread_create(&hiloConsola, NULL, (void*) escucharConsola, NULL);
	pthread_create(&hiloPrincipal, NULL, (void*) iniciarPlanificador, NULL);
	pthread_create(&hiloPlanificacion, NULL, (void*) ejecutarPlanificacion, NULL);

	pthread_join(hiloConsola, NULL);

	pthread_cancel(hiloPrincipal);
	pthread_cancel(hiloPlanificacion);

	liberarRecursos(EXIT_SUCCESS);
	return 0;
}

void print_header() {
	printf("\n\t\e[31;1m=========================================\e[0m\n");
	printf("\t.:: Bievenido a ReDistinto ::.");
	printf("\n\t\e[31;1m=========================================\e[0m\n\n");
}

int generarId() {
	return esi_id++;
}

int inicializar() {

	if (create_log() == EXIT_FAILURE)
		exit_gracefully(EXIT_FAILURE);

	if (cargarConfiguracion(PLANNER_CFG_FILE) < 0) {
		error_log("No se encontró el archivo de configuración");
		return -1;
	}

	float alpha = (float) planificador_setup.ALPHA / 100;
	setAlpha(alpha);

	setEstimacionInicial(planificador_setup.ESTIMACION_INICIAL);

	setAlgoritmo(planificador_setup.ALGORITMO_PLANIFICACION);

	return 0;
}

void escucharConsola() {
	info_log("Se inicio hilo con la consola");

	// Se debe conectar con el Coordinador

	conectarseConCoordinadorConsola();

	while (true) {
		if (consolaLeerComando(console_log) == TERMINAR_CONSOLA) {
			pthread_exit(0);
			return;
		}
	}
}

void iniciarPlanificador() {
	conectarseConCoordinador();

	create_tcp_server();

	tcpserver_run(server, before_tpc_server_cycle, on_server_accept,
			on_server_read, on_server_command);

	pthread_exit(0);
}

void ejecutarPlanificacion() {
	while (true) {
		int cantidadDeEsis;
		sem_getvalue(&sem_esis, &cantidadDeEsis);
		
		// Hay algun esi listo para ejecutar?
		if (cantidadDeEsis == 0) {
			info_log("**************** NO HAY ESI *******************");
		}

		sem_wait(&sem_esis);

		info_log("\n\n **************** HAY ESI *******************\n");

		aplicar_algoritmo_planificacion();

		if (esiEjecutando != NULL) {
			log_info(console_log,"ESI actual\tid: %d \tTiempo estimado: %d\n",
					esiEjecutando->id, esiEjecutando->tiempoEstimado);

			bool resultado_comunicacion = ejecutarSiguienteESI(esiEjecutando->client_socket, esiEjecutando->socket_id);

			if(!resultado_comunicacion)
			{
				// Algo salio mal en la comunicacion. Aborto el ESI
				terminarEsiActual();
			}
			else
			{
				// Si le pudo indicar al esi que continue,
				// entonces continuo con la planificacion

				// Escucho al coordinador
				escucharCoordinador();

				// ACA LE TENGO QUE ESPERAR AL ESTADO DEL ESI
				int estado = esperarEstadoDelEsi(esiEjecutando->client_socket,
						esiEjecutando->socket_id);

				switch (estado) {
				case -1:
					info_log("Error al pedir el estado del ESI. Abortando ESI");
					terminarEsiActual();
					break;
				case ESI_IDLE:
					info_log("El ESI puede seguir ejecutando");

					// Aumento contadores esi actual
					esiEjecutando->tiempoRafagaActual++;

					// Por ahora no decremento el tiempo estimado.
					/*esiEjecutando->tiempoEstimado--;

				if(esiEjecutando->tiempoEstimado < 0)
					esiEjecutando->tiempoEstimado = 0;*/

					//Incremento el contador porque este esi todavia no salio del programa
					sem_post(&sem_esis);

					break;
				case ESI_BLOCKED:
					// En un bloqueo se supone que el esi no pudo ejecutar
					// por eso no hago el incremento de contadores
					info_log("El ESI esta bloqueado");
					break;
				case ESI_FINISHED:
					info_log("El ESI termino");

					liberarRecursosDeEsiFinalizado(esiEjecutando);
					tcpserver_remove_client(server, esiEjecutando->socket_id);
					terminarEsiActual();

					break;
				}
			}
		}

		// Realizar incrementos de contadores
		nuevoCicloDeCPU();
	}
	pthread_exit(0);
}

void create_tcp_server() {
	server = tcpserver_create(planificador_setup.NOMBRE_INSTANCIA, console_log,
			planificador_setup.CANTIDAD_MAXIMA_CLIENTES,
			planificador_setup.TAMANIO_COLA_CONEXIONES,
			planificador_setup.PUERTO_ESCUCHA_CONEXIONES, true);
	if (server == NULL) {
		error_log("Could not create TCP server. Aborting execution.");
		exit_gracefully(EXIT_FAILURE);
	}
}

void conectarseConCoordinador() {
	info_log("Conectando al Coordinador ...");

	coordinator_socket = connect_to_server(planificador_setup.IP_COORDINADOR,
			planificador_setup.PUERTO_COORDINADOR, console_log);

	if (coordinator_socket <= 0) {
		exit_gracefully(EXIT_FAILURE);
	}

	if (!perform_connection_handshake(coordinator_socket,
			planificador_setup.NOMBRE_INSTANCIA, PLANNER, console_log)) {
		exit_gracefully(EXIT_FAILURE);
	}
	info_log("Conexion exitosa al Coordinador.");
}

void conectarseConCoordinadorConsola() {
	info_log("Conectando al Coordinador mediante consola");

	coordinator_socket_console = 0;

	coordinator_socket_console = connect_to_server(planificador_setup.IP_COORDINADOR,
			planificador_setup.PUERTO_COORDINADOR_CONSOLA , console_log);

	if (coordinator_socket_console <= 0) {
		exit_gracefully(EXIT_FAILURE);
	}

	if (!perform_connection_handshake(coordinator_socket_console,
			planificador_setup.NOMBRE_INSTANCIA, PLANNER, console_log)) {
		exit_gracefully(EXIT_FAILURE);
	}
	info_log("Conexion exitosa al Coordinador mediante la consola");
}

bool ejecutarSiguienteESI(int esi_socket, int socket_id) {
	bool pudo_ejecutar = true;
	t_planner_execute_request planner_request;
	strcpy(planner_request.planner_name, planificador_setup.NOMBRE_INSTANCIA);

	void *buffer = serialize_planner_execute_request(&planner_request);

	int result = send(esi_socket, buffer, PLANNER_REQUEST_SIZE, 0);

	if (result <= 0) {
		error_log("Fallo al enviar instruccion de seguir ejecutando");
		tcpserver_remove_client(server, socket_id);
		pudo_ejecutar = false;
	}
	free(buffer);
	return pudo_ejecutar;
}

int esperarEstadoDelEsi(int esi_socket, int socket_id) {
	int esi_status = -1, bytesReceived = 0;
	void *res_buffer = malloc(ESI_STATUS_RESPONSE_SIZE);
	t_esi_status_response *esi_status_response = NULL;

	bytesReceived = recv(esi_socket, res_buffer, ESI_STATUS_RESPONSE_SIZE,
			MSG_WAITALL);

	if (bytesReceived < ESI_STATUS_RESPONSE_SIZE) {
		error_log("Error receiving status from ESI!");

		log_info(console_log,"Bytes leidos: %d | Esperados: %d",
				bytesReceived, ESI_STATUS_RESPONSE_SIZE);

		free(res_buffer);
		tcpserver_remove_client(server, socket_id);
		return esi_status;
	}

	esi_status_response = deserialize_esi_status_response(res_buffer);

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

	t_connection_header *connection_header = NULL;
	t_ack_message ack_message;
	void *ack_buffer;
	void *header_buffer = malloc(CONNECTION_HEADER_SIZE);


	/*************************** LEER EL HANDSHAKE *********************************/
	int res = recv(client_socket, header_buffer, CONNECTION_HEADER_SIZE, MSG_WAITALL);

	if (res <= 0) {
		error_log("¡Error en el handshake del cliente!");
		tcpserver_remove_client(server, socket_id);
		free(header_buffer);
		return;
	}

	connection_header = deserialize_connection_header(header_buffer);

	info_log_param1("Se recibio handshake del cliente: %s", connection_header->instance_name);


	/*************************** RESPONDER AL HANDSHAKE *********************************/
	strcpy(ack_message.instance_name, planificador_setup.NOMBRE_INSTANCIA);

	ack_buffer = serialize_ack_message(&ack_message);

	if (send(client_socket, ack_buffer, ACK_MESSAGE_SIZE, 0)
			!= ACK_MESSAGE_SIZE) {
		error_log("Could not send handshake acknowledge to TCP client.");
		tcpserver_remove_client(server, socket_id);
	} else {
		info_log_param1("Successfully connected to TCP Client: %s",
				connection_header->instance_name);
	}


	/*************************** SI EL HANDSHAKE LO HIZO UN ESI *********************************/
	if (connection_header->instance_type == ESI) {
		info_log("************* NUEVO ESI ***************");

		int id_esi = generarId();
		ESI_STRUCT * esi = nuevoESI(id_esi, client_socket, socket_id);
		agregarNuevoEsi(esi);
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
	void *res_buffer = malloc(COORDINATOR_OPERATION_REQUEST_SIZE);

	bytesReceived = recv(coordinator_socket, res_buffer, COORDINATOR_OPERATION_REQUEST_SIZE,
	MSG_WAITALL);

	if (bytesReceived < COORDINATOR_OPERATION_REQUEST_SIZE) {
		error_log("¡Error en la comunicacion con el coordinador!");

		log_info(console_log,"Bytes leidos: %d | Esperados: %d", bytesReceived,
				COORDINATOR_OPERATION_REQUEST_SIZE);

		free(res_buffer);
		return;
	}

	t_coordinator_operation_request *request =
			deserialize_coordinator_operation_request(res_buffer);

	log_info(console_log,"El coordinador solicita: %s", operacionAString(request->operation_type));

	char * key = &request->key[0];

	// 1 - Me fijo si el recurso esta bloqueado por otro esi
	int result = estaBloqueadoPor(esiEjecutando, key);

	switch (request->operation_type) {
		case GET: {
			if(result == 0)
			{
				// Esta bloqueado por otro esi
				info_log("BLOQUEO: El recurso esta bloqueado por otro ESI");
				bloquearEsiActual(key);
				responderCoordinador(coordinator_socket, OP_BLOCKED);
			}
			else if(result == 1) {
				// Ya esta bloqueado por el esi actual
				info_log("ERROR: El recurso ya esta bloqueado por el ESI actual");
				responderCoordinador(coordinator_socket, OP_ERROR);
			}
			else {
				// No esta bloqueado
				info_log("OK: El recurso no esta bloqueado por otro ESI, lo bloqueo");
				bloquearRecurso(key);
				responderCoordinador(coordinator_socket, OP_SUCCESS);
			}

			break;
		}
		case SET:{
			if(result == 0)
			{
				// Esta bloqueado por otro esi
				info_log("BLOQUEO: El recurso esta bloqueado por otro ESI");
				bloquearEsiActual(key);
				responderCoordinador(coordinator_socket, OP_BLOCKED);
			}
			else if(result == 1) {
				// Ya esta bloqueado por el esi actual
				info_log("OK: El recurso esta bloqueado por el ESI actual");
				responderCoordinador(coordinator_socket, OP_SUCCESS);
			}
			else {
				info_log("ERROR: El recurso no esta bloqueado por el ESI actual");
				responderCoordinador(coordinator_socket, OP_ERROR);
			}
			break;
		}
		case STORE: {
			if(result == 0)
			{
				// Esta bloqueado por otro esi
				info_log("BLOQUEO: El recurso esta bloqueado por otro ESI");
				responderCoordinador(coordinator_socket, OP_BLOCKED);
			}
			else if(result == 1) {
				// Ya esta bloqueado por el esi actual
				info_log("OK: El recurso esta bloqueado por el ESI actual, lo libero");
				liberarRecurso(key);
				responderCoordinador(coordinator_socket, OP_SUCCESS);
			}
			else {
				// No esta bloqueado
				info_log("ERROR: El recurso no esta bloqueado por el ESI actual");
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

void responderCoordinador(int socket, operation_result_e result){
	t_operation_response response;
	response.operation_result = result;

	void *buffer = serialize_operation_response(&response);

	int r = send(socket, buffer, OPERATION_RESPONSE_SIZE, 0);

	if (r <= 0) {
		error_log("No se pudo enviar la respuesta al coordinador");
	}
	free(buffer);
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
	pthread_mutex_destroy(&mutexPlanificador);

	sem_destroy(&sem_esis);

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
