#include "esi.h"
#include <stdbool.h>
#include <commons/string.h>
#include <sys/socket.h> // Para crear sockets, enviar, recibir, etc
#include <netdb.h> // Para getaddrinfo
#include <unistd.h> // Para close
#include "libs/protocols.h"
#include "libs/textfile.h"

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
	printf("\n\t\e[31;1m=================================================\e[0m");
	printf("\n\t.::           Bienvenido a ReDistinto         ::.");
	printf("\n\t.:: Ejecutor de Sentencias Interactivas (ESI) ::.");
	printf("\n\t\e[31;1m=================================================\e[0m\n\n");
}

void print_goodbye(){
	printf("\n\t\e[31;1m=================================================\e[0m\n");
	printf("\t.::      Gracias por utilizar ReDistinto      ::.");
	printf("\n\t\e[31;1m=================================================\e[0m\n\n");
}


void create_log(){
	esi_log = log_create("esi.log", "ReDistinto-ESI", true, LOG_LEVEL_TRACE);

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

void free_instruction_split(char **parts){
	for(int i = 0; parts[i] != NULL; i++){
		free(parts[i]);
	}
	free(parts);
}

t_program_instruction* parse_instruction(char *line){
	line[strlen(line)-1] = '\0';

	char** parts = string_n_split(line, 3, " ");
	operation_type_e operation_type;

	// parse operation type
	if(strcmp("GET", parts[0]) == 0){
		operation_type = GET;
	} else if (strcmp("SET", parts[0]) == 0){
		operation_type = SET;
	} else if (strcmp("STORE", parts[0]) == 0){
		operation_type = STORE;
	} else {
		log_error(esi_log, "Invalid operation type: %s found. Program cannot be executed. Culprit line: %s", parts[0], line);
		printf("\t\e[31;1m ERROR:\e[0m Operación inválida: %s en línea: %s.", parts[0], line);
		free_instruction_split(parts);
		return NULL;
	}

	// validate key
	if(parts[1] == NULL || strlen(parts[1]) == 0 || strlen(parts[1]) > MAX_KEY_LENGTH){
		log_error(esi_log, "Invalid key: %s found. Program cannot be executed. Culprit line: %s", parts[1], line);
		printf("\t\e[31;1m ERROR:\e[0m Clave inválida: %s en línea: %s.", parts[1], line);
		free_instruction_split(parts);
		return NULL;
	}

	// validate value
	if(operation_type == SET){
		if(parts[2] == NULL || strlen(parts[2]) == 0){
			log_error(esi_log, "Instruction SET without value found. Program cannot be executed. Culprit line: %s", line);
			printf("\t\e[31;1m ERROR:\e[0m Operación SET sin valor en línea: %s.", line);
			free_instruction_split(parts);
			return NULL;
		}
	}

	t_program_instruction *instruction = malloc(sizeof(t_program_instruction));
	instruction->operation_type = operation_type;
	strcpy(instruction->key, parts[1]);
	if(operation_type == SET){
		instruction->value = malloc(strlen(parts[2])+1);
		strcpy(instruction->value, parts[2]);
		instruction->value_size = strlen(parts[2])+1;
	} else {
		instruction->value = NULL;
		instruction->value_size = 0;
	}

	free_instruction_split(parts);
	return instruction;

}

t_queue* parse_program_instructions(char *program_filename){
	t_textfile* program_file = textfile_open(program_filename, "r");
	if(!program_file->open){
		printf("\t\e[31;1m ERROR:\e[0m No se pudo leer el archivo: %s.", program_filename);
		log_error(esi_log, "Could not read program file: %s.", program_filename);
		textfile_destroy(program_file);
		exit_gracefully(EXIT_FAILURE);
	}

	t_queue* instructions = queue_create();
	bool parse_errors = false;

	void execute_by_line(char* line){
		t_program_instruction *inst = parse_instruction(line);
		if(inst != NULL){
			queue_push(instructions, inst);
		} else {
			parse_errors = true;
		}
	}

	textfile_execute_by_line(program_file, PROGRAM_LINE_MAX_LENGTH, execute_by_line);

	if(parse_errors){
		printf("\t\e[31;1m ERROR:\e[0m El archivo %s tiene errores y no puede ser ejecutado.", program_filename);
		log_error(esi_log, "Program file has errors. Aborting execution");
		exit_gracefully(EXIT_FAILURE);
	}

	return instructions;
}

bool wait_for_planner_signal(){
	log_info(esi_log, "Waiting for Planner to signal next execution...");

	void *buffer = malloc(PLANNER_REQUEST_SIZE);

	if (recv(planner_socket, buffer, PLANNER_REQUEST_SIZE, MSG_WAITALL) < PLANNER_REQUEST_SIZE) {
		log_error(esi_log, "Error receiving planner request. Aborting execution.");
		free(buffer);
		return false;
	}

	t_planner_execute_request *planner_request = deserialize_planner_execute_request(buffer);

	log_info(esi_log, "Received signal from planner: %s.", planner_request->planner_name);

	free(buffer);
	free(planner_request);
	return true;
}


void destroy_program_instruction(void* instruction){
	if(((t_program_instruction*)instruction)->value != NULL){
		free(((t_program_instruction*)instruction)->value);
	}
	free(instruction);
}

operation_result_e coordinate_operation(t_program_instruction *instruction){
	t_operation_request esi_operation_request;
	strcpy(esi_operation_request.key, instruction->key);
	esi_operation_request.operation_type = instruction->operation_type;
	esi_operation_request.payload_size = instruction->value_size;

	log_trace(esi_log, "Sending operation request to Coordinator ...");

	void *req_buffer = malloc(OPERATION_REQUEST_SIZE);
	req_buffer = serialize_operation_request(&esi_operation_request);

	int result = send(coordinator_socket, req_buffer, OPERATION_REQUEST_SIZE, 0);
	if (result < OPERATION_REQUEST_SIZE) {
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

	void *res_buffer = malloc(OPERATION_RESPONSE_SIZE);

	operation_result_e operation_result;

	if (recv(coordinator_socket, res_buffer, OPERATION_RESPONSE_SIZE, MSG_WAITALL) < OPERATION_RESPONSE_SIZE) {
		log_error(esi_log, "Error receiving Coordinator response.");
		operation_result = OP_ERROR;

	} else {
		t_operation_response *coordinator_response = deserialize_operation_response(res_buffer);
		operation_result = coordinator_response->operation_result;
		log_info(esi_log, "Received Coordinator response: %i.", coordinator_response->operation_result);
		free(coordinator_response);
	}

	free(res_buffer);
	return operation_result;
}

void log_instruction(t_program_instruction *instr){
	char* operationType;
	switch(instr->operation_type){
		case GET:
			operationType = "GET";
			break;
		case SET:
			operationType = "SET";
			break;
		case STORE:
			operationType = "STORE";
			break;
	}

	char *value = instr->value != NULL ? instr->value : "";

	log_info(esi_log, "Executing instruction: %s %s %s", operationType, instr->key, value);
}

void execute_program(char *program_filename){
	t_queue* instructions = parse_program_instructions(program_filename);

	t_program_instruction* next_instruction;
	operation_result_e operation_result;

	while(queue_size(instructions) > 0){
		if(!wait_for_planner_signal()){
			queue_destroy_and_destroy_elements(instructions, destroy_program_instruction);
			exit_gracefully(EXIT_FAILURE);
		}

		next_instruction = (t_program_instruction* ) queue_peek(instructions);
		log_instruction(next_instruction);


		operation_result =  coordinate_operation(next_instruction);

		if(operation_result == OP_ERROR){
			log_error(esi_log, "There was an error performing the current operation. Type:%d. Key: %s.",
					next_instruction->operation_type, next_instruction->key);
			queue_destroy_and_destroy_elements(instructions, destroy_program_instruction);
			exit_gracefully(EXIT_FAILURE);
		} else if (operation_result == OP_SUCCESS){
			queue_pop(instructions);
			destroy_program_instruction(next_instruction);

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

	queue_destroy(instructions);

	//TODO: Cuando termina la ejecución del programa, cierra el socket y termina dando un error del lado del Planificador
	//      Habría que encontrar una mejor manera para que le planificador se entere de que terminó, y no sea por error.
	//      Puede ser con un mensaje nuevo (que el ESI espere a que el planificador reciba el FINISHED y le mande un ACK)
	//      O con un nuevo estado del ESI.
}

// TODO: Recibir path del archivo con el programa.
int main(int argc, char **argv) {
	char* program_filename;

	print_header();

	if(argc != 2){
		printf("\t\e[31;1m ERROR:\e[0m Debe proveer como único parámetro el path del archivo con el programa a correr.");
		exit_gracefully(EXIT_SUCCESS);
	}

	program_filename =  argv[1];


	create_log();

	load_config();


	//connect_with_coordinator();

	//connect_with_planner();


	execute_program(program_filename);

	log_info(esi_log, "Finished execution successfully.");

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

	print_goodbye();

	exit(retVal);
}
