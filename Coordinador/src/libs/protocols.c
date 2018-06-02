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


t_connection_header* deserialize_connection_header(void *buffer){
	t_connection_header* header = malloc(sizeof(t_connection_header));
	int lastIndex = 0;
	int instance_type = 0;

	deserialize_data(&instance_type, 4, buffer, &lastIndex);
	header->instance_type = instance_type;
	deserialize_data(&(header->instance_name), 31, buffer, &lastIndex);

	return header;
}



t_esi_status_response* deserialize_esi_status_response(void *buffer){
	t_esi_status_response* response = malloc(sizeof(t_esi_status_response));
	int lastIndex = 0;

	deserialize_data(&(response->status), 4, buffer, &lastIndex);
	deserialize_data(&(response->instance_name), 31, buffer, &lastIndex);

	return response;
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

void* serialize_operation_request(t_esi_operation_request *request){
	void* buffer = malloc(OPERATION_REQUEST_SIZE);
	int lastIndex = 0;

	serialize_data(&(request->operation_type), 4, &buffer, &lastIndex);
	serialize_data(&(request->key), 41, &buffer, &lastIndex);
	serialize_data(&(request->payload_size), 4, &buffer, &lastIndex);

	return buffer;
}

t_esi_operation_request* deserialize_operation_request(void *buffer){
	t_esi_operation_request* request = malloc(sizeof(t_esi_operation_request));
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


void* serialize_coordinador_request(t_coordinador_request *request){
	void* buffer = malloc(PLANNER_REQUEST_SIZE);
	int lastIndex = 0;

	serialize_data(&(request->coordinador_name), 31, &buffer, &lastIndex);

	return buffer;
}

void* serialize_coordinador_request_instancia(t_coordinador_request_instancia *request){
	void* buffer = malloc(INSTANCIA_REQUEST_SIZE);
	int lastIndex = 0;

	serialize_data(&(request->coordinador_name), 31, &buffer, &lastIndex);
	serialize_data(&(request->CANTIDAD_ENTRADAS), 4, &buffer, &lastIndex);
	serialize_data(&(request->CANTIDAD_ENTRADAS), 4, &buffer, &lastIndex);
	return buffer;
}
