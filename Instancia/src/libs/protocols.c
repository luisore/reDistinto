/*
 * protocols.c
 *
 *  Created on: 20 abr. 2018
 *      Author: avinocur
 */
#include "protocols.h"
#include <sys/socket.h>
#include <netdb.h> // Para getaddrinfo
#include <unistd.h> // Para close
#include <string.h>
#include <arpa/inet.h>

int connect_to_server(char *ip, int port, t_log *logger) {
	struct sockaddr_in server;

	server.sin_addr.s_addr = inet_addr(ip);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	int server_socket = socket(AF_INET , SOCK_STREAM , 0);
	if (server_socket == -1) {
		log_error(logger, "Could not create socket to connect to server on IP: %s, PORT: %d. Aborting execution!", ip, port);
		return -1;
	}

	//Connect to remote server
	if (connect(server_socket , (struct sockaddr *)&server , sizeof(struct sockaddr_in)) < 0) {
		if(server_socket != 0) close(server_socket);
		log_error(logger, "Could not connect to server on IP: %s, PORT: %d. Aborting execution!", ip, port);
		return -1;
	}

	log_info(logger, "Connected to server IP: %s PORT: %d", ip, port);
	return server_socket;
}

bool send_connection_header(int server_socket, char* instance_name,
		instance_type_e instance_type, t_log *logger){

	t_connection_header *connection_header = malloc(sizeof(t_connection_header));
	strcpy(connection_header->instance_name, instance_name);
	connection_header->instance_type = instance_type;

	char *buffer = serialize_connection_header(connection_header);

	log_trace(logger, "Sending connection header message...");
	int result = send(server_socket, buffer, CONNECTION_HEADER_SIZE, 0);

	free(buffer);
	free(connection_header);

	if (result <= 0) {
		log_error(logger, "Could not send connection header to server.");
		return false;
	}

	return true;
}

bool wait_for_acknowledge(int server_socket, t_log *logger){
	void* ack_buffer = malloc(ACK_MESSAGE_SIZE);

	if (recv(server_socket, ack_buffer, ACK_MESSAGE_SIZE, MSG_WAITALL) <= 0) {
		log_error(logger, "Error receiving handshake response. Aborting execution.");
		free(ack_buffer);
		return false;
	}

	t_ack_message *ack_message = deserialize_ack_message(ack_buffer);

	log_info(logger, "Handshake successful with server: %s.", ack_message->instance_name);

	free(ack_buffer);
	free(ack_message);

	return true;
}

bool perform_connection_handshake(int server_socket, char* instance_name,
	instance_type_e instance_type, t_log *logger){

	if(!send_connection_header(server_socket, instance_name, instance_type, logger)){
		return false;
	}

	log_trace(logger, "Handshake message sent. Waiting for response...");

	if(wait_for_acknowledge(server_socket, logger)){
		log_info(logger, "Handshake successful.");
		return true;
	} else {
		log_error(logger, "Handshake failed!");
		return false;
	}

}

void* serialize_connection_header(t_connection_header *header){
	void* buffer = malloc(CONNECTION_HEADER_SIZE);
	int lastIndex = 0;
	int instance_type = header->instance_type;

	serialize_data(&instance_type, 4, &buffer, &lastIndex);
	serialize_data(&(header->instance_name), 31, &buffer, &lastIndex);

	return buffer;
}

t_connection_header* deserialize_connection_header(void *buffer){
	t_connection_header* header = malloc(sizeof(t_connection_header));
	int lastIndex = 0;
	int instance_type = 0;

	deserialize_data(&instance_type, 4, buffer, &lastIndex);
	header->instance_type = instance_type;
	deserialize_data(&(header->instance_name), 31, buffer, &lastIndex);

	return header;
}

void* serialize_ack_message(t_ack_message *ack_message){
	void* buffer = malloc(ACK_MESSAGE_SIZE);
	int lastIndex = 0;

	serialize_data(&(ack_message->instance_name), 31, &buffer, &lastIndex);

	return buffer;
}

t_ack_message* deserialize_ack_message(void* buffer){
	t_ack_message* message = malloc(sizeof(t_ack_message));
	int lastIndex = 0;

	deserialize_data(&(message->instance_name), 31, buffer, &lastIndex);

	return message;
}

void* serialize_init_instancia_message(t_instance_init_values *init_values_message){
	void* buffer = malloc(INSTANCE_INIT_VALUES_SIZE);
	int lastIndex = 0;

	serialize_data(&(init_values_message->entry_size), 4, &buffer, &lastIndex);
	serialize_data(&(init_values_message->number_of_entries), 4, &buffer, &lastIndex);

	return buffer;
}

t_instance_init_values* deserialize_init_instancia_message(void* buffer){
	t_instance_init_values* message = malloc(sizeof(t_instance_init_values));
	int lastIndex = 0;

	deserialize_data(&(message->entry_size), 4, buffer, &lastIndex);
	deserialize_data(&(message->number_of_entries), 4, buffer, &lastIndex);

	return message;
}

void* serialize_operation_request(t_operation_request *request){
	void* buffer = malloc(OPERATION_REQUEST_SIZE);
	int lastIndex = 0;

	serialize_data(&(request->operation_type), 4, &buffer, &lastIndex);
	serialize_data(&(request->key), 41, &buffer, &lastIndex);
	serialize_data(&(request->payload_size), 4, &buffer, &lastIndex);

	return buffer;
}

t_operation_request* deserialize_operation_request(void *buffer){
	t_operation_request* request = malloc(sizeof(t_operation_request));
	int lastIndex = 0;

	deserialize_data(&(request->operation_type), 4, buffer, &lastIndex);
	deserialize_data(&(request->key), 41, buffer, &lastIndex);
	deserialize_data(&(request->payload_size), 4, buffer, &lastIndex);

	return request;
}

void* serialize_operation_response(t_operation_response *response){
	void* buffer = malloc(OPERATION_RESPONSE_SIZE);
	int lastIndex = 0;

	serialize_data(&(response->operation_result), 4, &buffer, &lastIndex);

	return buffer;
}

t_operation_response* deserialize_operation_response(void *buffer){
	t_operation_response* response = malloc(sizeof(t_operation_response));
	int lastIndex = 0;

	deserialize_data(&(response->operation_result), 4, buffer, &lastIndex);

	return response;
}

void* serialize_planner_execute_request(t_planner_execute_request *request){
	void* buffer = malloc(PLANNER_REQUEST_SIZE);
	int lastIndex = 0;

	serialize_data(&(request->planner_name), 31, &buffer, &lastIndex);

	return buffer;
}

t_planner_execute_request* deserialize_planner_execute_request(void *buffer){
	t_planner_execute_request* request = malloc(sizeof(t_planner_execute_request));
	int lastIndex = 0;

	deserialize_data(&(request->planner_name), 31, buffer, &lastIndex);

	return request;
}

void* serialize_esi_status_response(t_esi_status_response *response){
	void* buffer = malloc(ESI_STATUS_RESPONSE_SIZE);
	int lastIndex = 0;

	serialize_data(&(response->status), 4, &buffer, &lastIndex);
	serialize_data(&(response->instance_name), 31, &buffer, &lastIndex);

	return buffer;
}

t_esi_status_response* deserialize_esi_status_response(void *buffer){
	t_esi_status_response* response = malloc(sizeof(t_esi_status_response));
	int lastIndex = 0;

	deserialize_data(&(response->status), 4, buffer, &lastIndex);
	deserialize_data(&(response->instance_name), 31, buffer, &lastIndex);

	return response;
}

void* serialize_coordinator_operation_request(t_coordinator_operation_request *request){
	void* buffer = malloc(COORDINATOR_OPERATION_REQUEST_SIZE);
	int lastIndex = 0;

	serialize_data(&(request->operation_type), 4, &buffer, &lastIndex);
	serialize_data(&(request->key), 41, &buffer, &lastIndex);

	return buffer;
}

t_coordinator_operation_request* deserialize_coordinator_operation_request(void *buffer){
	t_coordinator_operation_request* request = malloc(sizeof(t_coordinator_operation_request));
	int lastIndex = 0;

	deserialize_data(&(request->operation_type), 4, buffer, &lastIndex);
	deserialize_data(&(request->key), 41, buffer, &lastIndex);

	return request;
}

void* serialize_instance_response(t_instance_response *response){
	void* buffer = malloc(INSTANCE_RESPONSE_SIZE);
	int lastIndex = 0;

	serialize_data(&(response->status), 4, &buffer, &lastIndex);
	serialize_data(&(response->payload_size), 4, &buffer, &lastIndex);

	return buffer;
}

t_instance_response* deserialize_instance_response(void *buffer){
	t_instance_response* response = malloc(sizeof(t_instance_response));
	int lastIndex = 0;

	deserialize_data(&(response->status), 4, buffer, &lastIndex);
	deserialize_data(&(response->payload_size), 4, buffer, &lastIndex);

	return response;
}

void* serialize_instance_init_values(t_instance_init_values *init_values){
	void* buffer = malloc(INSTANCE_INIT_VALUES_SIZE);
	int lastIndex = 0;

	serialize_data(&(init_values->entry_size), 4, &buffer, &lastIndex);
	serialize_data(&(init_values->number_of_entries), 4, &buffer, &lastIndex);

	return buffer;
}

t_instance_init_values* deserialize_instance_init_values(void *buffer){
	t_instance_init_values* init_values = malloc(sizeof(t_instance_response));
	int lastIndex = 0;

	deserialize_data(&(init_values->entry_size), 4, buffer, &lastIndex);
	deserialize_data(&(init_values->number_of_entries), 4, buffer, &lastIndex);

	return init_values;
}

void* serialize_coordinator_operation_header(t_coordinator_operation_header *header){
	void* buffer = malloc(COORDINATOR_OPERATION_HEADER_SIZE);
	int lastIndex = 0;

	serialize_data(&(header->coordinator_operation_type), 4, &buffer, &lastIndex);

	return buffer;
}

t_coordinator_operation_header* deserialize_coordinator_operation_header(void* buffer){
	t_coordinator_operation_header* header = malloc(sizeof(t_coordinator_operation_header));
	int lastIndex = 0;

	deserialize_data(&(header->coordinator_operation_type), 4, buffer, &lastIndex);

	return header;
}

void* serialize_status_response_from_coordinator(status_response_from_coordinator *response){

	void* buffer = malloc(STATUS_RESPONSE_FROM_COORDINATOR);
	int lastIndex = 0;

	serialize_data(&(response->payload_valor_size), 4, &buffer, &lastIndex);
	serialize_data(&(response->nombre_intancia_actual), 41, &buffer, &lastIndex);
	serialize_data(&(response->nombre_intancia_posible), 41, &buffer, &lastIndex);

	return buffer;

}

status_response_from_coordinator* derialize_status_response_from_coordinator(void *buffer){

	status_response_from_coordinator* response = malloc(sizeof(status_response_from_coordinator));
	int lastIndex = 0;

	deserialize_data(&(response->payload_valor_size), 4, buffer, &lastIndex);
	deserialize_data(&(response->nombre_intancia_actual), 41, buffer, &lastIndex);
	deserialize_data(&(response->nombre_intancia_posible), 41, buffer, &lastIndex);

	return response;

}

