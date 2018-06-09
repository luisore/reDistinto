#include "Planificador.h"

int main(void) {

	imprimirCabecera();

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

void imprimirCabecera() {
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
	conectarseConCoordinador();

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

		/*if(list_size(listaEsiNuevos) == 0 && list_size(listaEsiListos) == 0)
		{
			if(list_size(listaEsiBloqueados) != 0)
			{
				// Tengo que analizar si esto es lo que mas conviene
			}
		}*/

		aplicar_algoritmo_planificacion();

		if (esiEjecutando != NULL) {
			log_info(console_log, "ESI actual\tid: %d \tTiempo estimado: %d\n",
					esiEjecutando->id, esiEjecutando->tiempoEstimado);

			ejecutarSiguienteESI(esiEjecutando->client_socket, esiEjecutando->socket_id);

			// Escucho al coordinador
			escucharCoordinador();

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

				// Aumento contadores esi actual
				esiEjecutando->tiempoRafagaActual++;
				esiEjecutando->tiempoEstimado--;
				if(esiEjecutando->tiempoEstimado < 0)
					esiEjecutando->tiempoEstimado = 0;

				break;
			case ESI_BLOCKED:
				// En un bloqueo se supone que el esi no pudo ejecutar
				// por eso no hago el incremento de contadores
				log_info(console_log, "El ESI esta bloqueado");
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

void ejecutarSiguienteESI(int esi_socket, int socket_id) {
	t_planner_execute_request planner_request;
	strcpy(planner_request.planner_name, planificador_setup.NOMBRE_INSTANCIA);

	void *buffer = serialize_planner_execute_request(&planner_request);

	int result = send(esi_socket, buffer, PLANNER_REQUEST_SIZE, 0);

	if (result <= 0) {
		log_error(console_log, "Fallo al enviar instruccion de seguir ejecutando");
		tcpserver_remove_client(server, socket_id);
	}
	free(buffer);
}

int esperarEstadoDelEsi(int esi_socket, int socket_id) {
	int esi_status = -1, bytesReceived = 0;
	void *res_buffer = malloc(ESI_STATUS_RESPONSE_SIZE);
	t_esi_status_response *esi_status_response = NULL;

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

	esi_status_response = deserialize_esi_status_response(res_buffer);

	log_info(console_log, "Estado del ESI: %d", esi_status_response->status);

	esi_status = esi_status_response->status;

	free(res_buffer);
	free(esi_status_response);
	return esi_status;
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
		log_error(console_log, "¡Error en el handshake del cliente!");
		tcpserver_remove_client(server, socket_id);
		free(header_buffer);
		return;
	}

	connection_header = deserialize_connection_header(header_buffer);

	log_info(console_log, "Se recibio handshake del cliente: %s", connection_header->instance_name);


	/*************************** RESPONDER AL HANDSHAKE *********************************/
	strcpy(ack_message.instance_name, planificador_setup.NOMBRE_INSTANCIA);

	ack_buffer = serialize_ack_message(&ack_message);

	if (send(client_socket, ack_buffer, ACK_MESSAGE_SIZE, 0)
			!= ACK_MESSAGE_SIZE) {
		log_error(console_log,
				"Could not send handshake acknowledge to TCP client.");
		tcpserver_remove_client(server, socket_id);
	} else {
		log_info(console_log, "Successfully connected to TCP Client: %s",
				connection_header->instance_name);
	}


	/*************************** SI EL HANDSHAKE LO HIZO UN ESI *********************************/
	if (connection_header->instance_type == ESI) {
		log_info(console_log, "************* NUEVO ESI ***************");

		int id_esi = generarId();
		ESI_STRUCT * esi = nuevoESI(id_esi, client_socket, socket_id);
		agregarNuevoEsi(esi);

		// Nuevo esi => libero el hilo de planificacion
		pthread_mutex_unlock(&mutexPlanificacion);
	}

	free(header_buffer);
	free(connection_header);
	free(ack_buffer);
}

void before_tpc_server_cycle(tcp_server_t* server) {}
void on_server_read(tcp_server_t* server, int client_socket, int socket_id) {}
void on_server_command(tcp_server_t* server) {}



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
	liberarRecursosCoordinador();

	exit_gracefully(tipoSalida);
}

void imprimirFin() {
	printf("\n\t\e[31;1m=========================================\e[0m\n");
	printf("\t.:: Gracias por usar ReDistinto ::.");
	printf("\n\t\e[31;1m=========================================\e[0m\n\n");
}

void exit_gracefully(int retVal) {

	if (server != NULL)
		tcpserver_destroy(server);

	imprimirFin();

	exit(retVal);
}
