#include "esi.h"
#include <stdbool.h>
#include <commons/string.h>
#include <sys/socket.h> // Para crear sockets, enviar, recibir, etc
#include <netdb.h> // Para getaddrinfo
#include <unistd.h> // Para close
#include <commons/collections/queue.h>
#include "libs/protocols.h"

void exit_gracefully(int retVal);

void load_config() {
	log_trace(esi_log, "Loading configuration from file: %s", ESI_CFG_FILE);

	t_config *config = config_create(ESI_CFG_FILE);
	if(config == NULL){
		log_error(esi_log, "Could not load configuration file.");
		exit_gracefully(EXIT_FAILURE);
	}

	instance_name = string_duplicate(config_get_string_value(config, "instanceName"));
	coordinator_ip = string_duplicate(config_get_string_value(config, "coordinatorIP"));
	coordinator_port = config_get_int_value(config, "coordinatorPort");
	planner_ip = string_duplicate(config_get_string_value(config, "plannerIP"));
	planner_port = config_get_int_value(config, "plannerPort");

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
		exit_gracefully(EXIT_FAILURE);
	}
}

bool send_status_to_planner(esi_status_e esi_status){
	log_info(esi_log, "Sending status response to Planner...");

	t_esi_status_response esi_status_reponse;
	esi_status_reponse.status = esi_status;
	strcpy(esi_status_reponse.instance_name, instance_name);

	void *buffer = serialize_esi_status_response(&esi_status_reponse);

	int result = send(planner_socket, buffer, ESI_STATUS_RESPONSE_SIZE, 0);
	free(buffer);

	if (result < ESI_STATUS_RESPONSE_SIZE) {
		log_error(esi_log, "Could not send status response to Planner.");
		return false;
	}

	log_info(esi_log, "Status response sent to planner");
	return true;
}


void connect_with_coordinator() {
	log_info(esi_log, "Connecting to Coordinator.");
	coordinator_socket = connect_to_server(coordinator_ip, coordinator_port, esi_log);
	if(coordinator_socket <= 0){
		exit_gracefully(EXIT_FAILURE);
	}

	if(!perform_connection_handshake(coordinator_socket, instance_name, ESI, esi_log)){
		exit_gracefully(EXIT_FAILURE);
	}
	log_info(esi_log, "Successfully connected to Coordinator.");
}

void connect_with_planner() {
	log_info(esi_log, "Connecting to Planner.");
	planner_socket = connect_to_server(planner_ip, planner_port, esi_log);
	if(planner_socket <= 0){
		exit_gracefully(EXIT_FAILURE);
	}

	if(!perform_connection_handshake(planner_socket, instance_name, ESI, esi_log)){
		exit_gracefully(EXIT_FAILURE);
	}
	log_info(esi_log, "Successfully connected to Planner.");
}

t_queue* parse_program_instructions(){
	t_queue* instructions = queue_create();

	t_program_instruction *i1 = malloc(sizeof(t_program_instruction));
	strcpy(i1->key, "deportista:futbol");
	i1->operation_type = SET;
	i1->value = malloc(strlen("Lionel Messi")+1);
	strcpy(i1->value, "Lionel Messi");
	i1->value_size = strlen(i1->value) + 1;
	queue_push(instructions, i1);

	t_program_instruction *i2 = malloc(sizeof(t_program_instruction));
	strcpy(i2->key, "deportista:basket");
	i2->operation_type = SET;
	i2->value = malloc(strlen("Manu Ginobili")+1);
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
	if (!send_status_to_planner(ESI_IDLE)){
		log_error(esi_log, "Could not signal Planner to send the next instruction!");
		exit_gracefully(EXIT_FAILURE);
	}

	log_info(esi_log, "Waiting for Planner to signal next execution...");

	void *buffer = malloc(PLANNER_REQUEST_SIZE);

	if (recv(planner_socket, buffer, PLANNER_REQUEST_SIZE, MSG_WAITALL) < PLANNER_REQUEST_SIZE) {
		log_error(esi_log, "Error receiving planner request. Aborting execution.");
		free(buffer);
		return false;
	}

	t_planner_request *planner_request = deserialize_planner_request(buffer);

	log_info(esi_log, "Received signal from planner: %s.", planner_request->planner_name);

	free(buffer);
	free(planner_request);
	return true;
}


void destroy_program_instruction(t_program_instruction* instruction){
	if(instruction->value != NULL){
		free(instruction->value);
	}
	free(instruction);
}

operation_result_e coordinate_operation(t_program_instruction *instruction){
	t_esi_operation_request esi_operation_request;
	strcpy(esi_operation_request.key, instruction->key);
	esi_operation_request.operation_type = instruction->operation_type;
	esi_operation_request.payload_size = instruction->value_size;

	log_trace(esi_log, "Sending operation request to Coordinator ...");

	void *req_buffer = malloc(ESI_OPERATION_REQUEST_SIZE);
	req_buffer = serialize_esi_operation_request(&esi_operation_request);

	int result = send(coordinator_socket, req_buffer, ESI_OPERATION_REQUEST_SIZE, 0);
	if (result < ESI_OPERATION_REQUEST_SIZE) {
		free(req_buffer);
		log_error(esi_log, "Could not send operation request to Coordinator.");
		return OP_ERROR;
	}
	free(req_buffer);

	if(instruction->value_size > 0){
		log_trace(esi_log, "Sending payload to Coordinator ...");
		result = send(coordinator_socket, instruction->value, strlen(instruction->value)+1, 0);
		if (result <= 0) {
			log_error(esi_log, "Could not send payload to Coordinator.");
			return OP_ERROR;
		}
	}

	void *res_buffer = malloc(COORD_OPERATION_RESPONSE_SIZE);

	operation_result_e operation_result;

	if (recv(coordinator_socket, res_buffer, COORD_OPERATION_RESPONSE_SIZE, MSG_WAITALL) < COORD_OPERATION_RESPONSE_SIZE) {
		log_error(esi_log, "Error receiving Coordinator response.");
		operation_result = OP_ERROR;

	} else {
		t_coordinator_operation_response *coordinator_response = deserialize_coordinator_operation_response(res_buffer);
		operation_result = coordinator_response->operation_result;
		log_info(esi_log, "Received Coordinator response: %s.", coordinator_response->operation_result);
		free(coordinator_response);
	}

	free(res_buffer);
	return operation_result;
}

void execute_program(){
	t_queue* instructions = parse_program_instructions();

	t_program_instruction* next_instruction;
	operation_result_e operation_result;

	while(queue_size(instructions) > 0){
		if(!wait_for_planner_signal()){
			queue_destroy_and_destroy_elements(instructions, destroy_program_instruction);
			exit_gracefully(EXIT_FAILURE);
		}

		next_instruction = (t_program_instruction* ) queue_peek(instructions);

		operation_result = OP_SUCCESS; //coordinate_operation(next_instruction);

		if(operation_result == OP_ERROR){
			log_error(esi_log, "There was an error performing the current operation. Type:%d. Key: %s.",
					next_instruction->operation_type, next_instruction->key);
			queue_destroy_and_destroy_elements(instructions, destroy_program_instruction);
			exit_gracefully(EXIT_FAILURE);
		} else if (operation_result == OP_SUCCESS){
			queue_pop(instructions);
			free(next_instruction);

			if(!send_status_to_planner(queue_size(instructions) > 0 ? ESI_IDLE : ESI_FINISHED)){
				queue_destroy_and_destroy_elements(instructions, destroy_program_instruction);
				exit_gracefully(EXIT_FAILURE);
			}
		} else { // operation_result == OP_BLOCKED
			if(!send_status_to_planner(ESI_BLOCKED)){
				queue_destroy_and_destroy_elements(instructions, destroy_program_instruction);
				exit_gracefully(EXIT_FAILURE);
			}
		}
	}

	//TODO: Cuando termina la ejecución del programa, cierra el socket y termina dando un error del lado del Planificador
	//      Habría que encontrar una mejor manera para que le planificador se entere de que terminó, y no sea por error.
	//      Puede ser con un mensaje nuevo (que el ESI espere a que el planificador reciba el FINISHED y le mande un ACK)
	//      O con un nuevo estado del ESI.
}

// TODO: Recibir path del archivo con el programa.
int main(void) {
	print_header();

	create_log();

	load_config();

	//connect_with_coordinator();

	connect_with_planner();

	execute_program();

	log_info(esi_log, "Finished execution successfully.");

	print_goodbye();

	exit_gracefully(EXIT_SUCCESS);

	return 0;
}


void exit_gracefully(int retVal){
	if(instance_name != NULL) free(instance_name);
	if(coordinator_ip != NULL) free(coordinator_ip);
	if(planner_ip != NULL) free(planner_ip);
	if(esi_log != NULL) log_destroy(esi_log);

	if(coordinator_socket != 0) close(coordinator_socket);
	if(planner_socket != 0) close(planner_socket);

	exit(retVal);
}
