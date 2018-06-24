#include "Coordinador.h"

int coordinator_socket = 0;

int conectarseConCoordinador() {
	log_info(console_log, "Conectando al Coordinador ...");

	coordinator_socket = connect_to_server(planificador_setup.IP_COORDINADOR,
			planificador_setup.PUERTO_COORDINADOR, console_log);

	if (coordinator_socket <= 0) {
		return EXIT_FAILURE;
	}

	if (!perform_connection_handshake(coordinator_socket,
			planificador_setup.NOMBRE_INSTANCIA, PLANNER, console_log)) {
		return EXIT_FAILURE;
	}
	log_info(console_log, "Conexion exitosa al Coordinador.");
	return EXIT_SUCCESS;
}

void escucharCoordinador(){
	int bytesReceived = 0;
	void *res_buffer = malloc(COORDINATOR_OPERATION_REQUEST_SIZE);

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
				bloquearEsiActual(key);
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
				bloquearEsiActual(key);
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

void liberarRecursosCoordinador(){
	if (coordinator_socket != 0)
			close(coordinator_socket);
}
