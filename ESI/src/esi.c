#include "esi.h"
#include <stdbool.h>
#include <commons/string.h>
#include <sys/socket.h> // Para crear sockets, enviar, recibir, etc
#include <netdb.h> // Para getaddrinfo
#include <unistd.h> // Para close
#include <commons/collections/queue.h>
#include "libs/protocols.h"

void exitGacefully(int retVal);

void loadConfig() {
	log_trace(esi_log, "Loading configuration from file: %s", ESI_CFG_FILE);

	t_config *config = config_create(ESI_CFG_FILE);
	if(config == NULL){
		log_error(esi_log, "Could not load configuration file.");
		exitGacefully(EXIT_FAILURE);
	}

	instance_name = config_get_string_value(config, "instanceName");
	coordinator_ip = config_get_string_value(config, "coordinatorIP");
	coordinator_port = config_get_string_value(config, "coordinatorPort");
	planner_ip = config_get_string_value(config, "plannerIP");
	planner_port = config_get_string_value(config, "plannerPort");

	log_debug(esi_log, "Loaded configuration. Coordinator IP: %s PORT: %d Planner IP: %s PORT: %d",
			coordinator_ip, coordinator_port, planner_ip, planner_port);

	config_destroy(config);
}

void print_header(){
	printf("\n\t\e[31;1m=========================================\e[0m\n");
	printf("\t.:: Bienvenido a ReDistinto ::.");
	printf("\t.:: Ejecutor de Sentencias Interactivas (ESI) ::.");
	printf("\n\t\e[31;1m=========================================\e[0m\n\n");
}

void print_goodbye(){
	printf("\n\t\e[31;1m=========================================\e[0m\n");
	printf("\t.:: Gracias por utilizar ReDistinto ::.");
	printf("\n\t\e[31;1m=========================================\e[0m\n\n");
}


void create_log(){
	esi_log = log_create("esi.log", "ReDistinto-ESI", false, LOG_LEVEL_TRACE);

	if(esi_log == NULL){
		printf("Could not create log. Execution aborted.");
		exitGacefully(EXIT_FAILURE);
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

	freeaddrinfo(server_info);  // No lo necesitamos mas

	if (res < 0) {
		if(server_socket != 0) close(server_socket);
		exitGacefully(EXIT_FAILURE);
	}


	log_info(esi_log, "Connected to server IP: %s PORT: %s");
	return server_socket;
}

void perform_connection_handshake(int server_socket){
	t_connection_header connection_header;
	strcpy(connection_header.instance_name, instance_name);
	connection_header.instance_type = ESI;

	log_trace(esi_log, "Sending handshake message...");
	int result = send(server_socket, &connection_header, sizeof(t_connection_header), 0);

	if (result <= 0) {
		log_error(esi_log, "Could not perform handshake with server. Send message failed");
		exitGacefully(EXIT_FAILURE);
	}
	log_trace(esi_log, "Handshake message sent. Waiting for response...");

	t_ack_message * ack_message = malloc(sizeof(t_ack_message));

	if (recv(socket, ack_message, sizeof(t_ack_message), 0) <= 0) {
		log_error(esi_log, "Error receiving handshake response. Aborting execution.");
		exitGacefully(EXIT_FAILURE);
	}

	log_info(esi_log, "Handshake successful with server: %s.", ack_message->instance_name);

	free(ack_message);
}

void connect_with_coordinator() {
	log_info(esi_log, "Connecting to Coordinator.");
	coordinator_socket = connect_to_server(coordinator_ip, coordinator_port);

	perform_connection_handshake(coordinator_socket);
	log_info(esi_log, "Successfully connected to Coordinator.");
}

void connect_with_planner() {
	log_info(esi_log, "Connecting to Planner.");
	planner_socket = connect_to_server(planner_ip, planner_port);

	perform_connection_handshake(planner_socket);
	log_info(esi_log, "Successfully connected to Planner.");
}

t_queue* parse_program_instructions(){
	t_queue* instructions = queue_create();

	t_program_instruction *i1 = malloc(sizeof(t_program_instruction));
	strcpy(i1->key, "deportista:futbol");
	i1->operation_type = SET;
	strcpy(i1->value, "Lionel Messi");
	i1->value_size = strlen(i1->value) + 1;
	queue_push(instructions, i1);

	t_program_instruction *i2 = malloc(sizeof(t_program_instruction));
	strcpy(i2->key, "deportista:basket");
	i2->operation_type = SET;
	strcpy(i2->value, "Manu Ginoboli");
	i2->value_size = strlen(i2->value) + 1;
	queue_push(instructions, i2);

	t_program_instruction *i3 = malloc(sizeof(t_program_instruction));
	strcpy(i3->key, "deportista:futbol");
	i3->operation_type = STORE;
	i3->value = NULL;
	i3->value_size = 0;
	queue_push(instructions, i3);

	t_program_instruction *i4 = malloc(sizeof(t_program_instruction));
	strcpy(i4->key, "deportista:basket");
	i4->operation_type = STORE;
	i4->value = NULL;
	i4->value_size = 0;
	queue_push(instructions, i4);

	return instructions;
}

bool wait_for_planner_signal(){
	log_info(esi_log, "Waiting for Planner to signal next execution...");

	t_planner_request * planner_request = malloc(sizeof(t_planner_request));

	if (recv(socket, planner_request, sizeof(t_planner_request), 0) <= 0) {
		log_error(esi_log, "Error receiving planner request. Aborting execution.");
		return false;
	}

	log_info(esi_log, "Received signal from planner: %s.", planner_request->planner_name);
	return true;
}


void destroy_program_instruction(t_program_instruction* instruction){
	free(instruction->value);
	free(instruction);
}

enum operation_result_e coordinate_operation(t_program_instruction *instruction){
	t_esi_operation_request esi_operation_request;
	strcpy(esi_operation_request.key, instruction->key);

	esi_operation_request.operation_type = instruction->operation_type;
	esi_operation_request.payload_size = instruction->value_size;

	log_trace(esi_log, "Sending operation request to Coordinator ...");
	int result = send(coordinator_socket, &esi_operation_request, sizeof(t_esi_operation_request), 0);
	if (result <= 0) {
		log_error(esi_log, "Could not send operation request to Coordinator.");
		return OP_ERROR;
	}

	if(instruction->value_size > 0){
		log_trace(esi_log, "Sending payload to Coordinator ...");
		result = send(coordinator_socket, instruction->value, strlen(instruction->value)+1, 0);
		if (result <= 0) {
			log_error(esi_log, "Could not send payload to Coordinator.");
			return OP_ERROR;
		}
	}

	t_coordinator_operation_response *coordinator_response = malloc(sizeof(t_coordinator_operation_response));
	enum operation_result_e operation_result;

	if (recv(socket, coordinator_response, sizeof(t_coordinator_operation_response), 0) <= 0) {
		free(coordinator_response);
		log_error(esi_log, "Error receiving Coordinator response.");
		operation_result = OP_ERROR;
	} else {
		operation_result = coordinator_response->operation_result;
		log_info(esi_log, "Received Coordinator response: %s.", coordinator_response->operation_result);
	}

	free(coordinator_response);
	return operation_result;
}

bool send_response_to_planner(enum esi_status_e esi_status){
	t_esi_status_response esi_status_reponse;
	esi_status_reponse.status = esi_status;

	log_info(esi_log, "Sending status response to Planner...");

	int result = send(coordinator_socket, &esi_status_reponse, sizeof(t_esi_status_response), 0);
	if (result <= 0) {
		log_error(esi_log, "Could not send status response to Planner.");
		return false;
	}

	log_info(esi_log, "Status response sent to planner");
	return true;
}

void execute_program(){
	t_queue* instructions = parse_program_instructions();

	t_program_instruction* next_instruction;
	enum operation_result_e operation_result;

	while(queue_size(instructions) > 0){
		if(!wait_for_planner_signal()){
			queue_destroy_and_destroy_elements(instructions, destroy_program_instruction);
			exitGacefully(EXIT_FAILURE);
		}

		next_instruction = (t_program_instruction* ) queue_peek(instructions);

		operation_result = coordinate_operation(next_instruction);

		if(operation_result == OP_ERROR){
			log_error(esi_log, "There was an error performing the current operation. Type:%d. Key: %s.",
					next_instruction->operation_type, next_instruction->key);
			queue_destroy_and_destroy_elements(instructions, destroy_program_instruction);
			exitGacefully(EXIT_FAILURE);
		} else if (operation_result == OP_SUCCESS){
			queue_pop(instructions);
			free(next_instruction);

			if(!send_response_to_planner(queue_size(instructions) > 0 ? ESI_IDLE : ESI_FINISHED)){
				queue_destroy_and_destroy_elements(instructions, destroy_program_instruction);
				exitGacefully(EXIT_FAILURE);
			}
		} else { // operation_result == OP_BLOCKED
			if(!send_response_to_planner(ESI_BLOCKED)){
				queue_destroy_and_destroy_elements(instructions, destroy_program_instruction);
				exitGacefully(EXIT_FAILURE);
			}
		}
	}
}

int main(void) {
	print_header();

	create_log();

	loadConfig();

	connect_with_coordinator();

	connect_with_planner();

	execute_program();

	log_info(esi_log, "Finished execution successfully.");

	print_goodbye();

	exitGacefully(EXIT_SUCCESS);

	return 0;
}


void exitGacefully(int retVal){
	if(instance_name != NULL) free(instance_name);
	if(coordinator_ip != NULL) free(coordinator_ip);
	if(coordinator_port != NULL) free(coordinator_port);
	if(planner_ip != NULL) free(planner_ip);
	if(planner_port != NULL) free(planner_port);
	if(esi_log != NULL) log_destroy(esi_log);

	if(coordinator_socket != 0) close(coordinator_socket);
	if(planner_socket != 0) close(planner_socket);

	exit(retVal);
}
