/*
 * protocols.h
 *
 *  Created on: 18 abr. 2018
 *      Author: avinocur
 */

#ifndef _PROTOCOLS_H_
#define _PROTOCOLS_H_
#include <stdbool.h>
#include <commons/log.h>

#include "../library/serialize.h"

typedef enum {
	ESI = 1, COORDINATOR = 2, PLANNER = 3, REDIS_INSTANCE = 4
} instance_type_e;

typedef enum {
	GET = 1, SET = 2, STORE = 3
} operation_type_e;

typedef enum {
	ESI_IDLE = 1, ESI_BLOCKED = 2, ESI_FINISHED = 3
} esi_status_e;

typedef enum {
	KEY_OPERATION = 1, COMPACT = 2
} coordinator_operation_type_e;

/*
 * Result of the operation performed by the Coordinator. It can be:
 * OP_SUCCESS = The requested operation was performed.
 * OP_ERROR   = There was an error with the requested key.
 * OP_BLOCKED = The requested key is blocked by another ESI.
 */
typedef enum {
	OP_SUCCESS = 1, OP_ERROR = 2, OP_BLOCKED = 3
} operation_result_e;

/*
 * Status of the instance after executing the operation:
 * INSTANCE_SUCCESS: The operation completed successfully.
 * INSTANCE_ERROR: The operation failed.
 * INSTANCE_COMPACT: The instance needs to compact and the operation should be
 * resubmitted after the compaction.
 */
typedef enum {
	INSTANCE_SUCCESS = 1, INSTANCE_ERROR = 2, INSTANCE_COMPACT = 3
} instance_status_e;

/*
 * Header used in every connection between processes.
 * Either when connecting for the first time or before sending any message.
 */
typedef struct {
	char instance_name[30];
	instance_type_e instance_type;
} t_connection_header;

static const int CONNECTION_HEADER_SIZE = 31 + 4;

typedef struct {
	char instance_name[30];
} t_ack_message;

static const int ACK_MESSAGE_SIZE = 31;

static const int ESI_INSTRUCTION_REQUEST_SIZE = 31 + 4;

/*
 * ESI Operation to send to the Coordinator
 * Coordinator Operation to send to the Instance
 */
typedef struct {
	operation_type_e operation_type;
	char key[40];
	unsigned int payload_size;
} t_operation_request;

static const int OPERATION_REQUEST_SIZE = 4 + 41 + 4;

/*
 * Coordinator response with the operation result
 */
typedef struct {
	operation_result_e operation_result;
} t_operation_response;

static const int OPERATION_RESPONSE_SIZE = 4;

/*
 * Planner request to execute next instruction
 */
typedef struct {
	char planner_name[30]; // En realidad no importa lo que mandes!
} t_planner_execute_request;

static const int PLANNER_REQUEST_SIZE = 31;

/*
 * ESI response with the current status
 */
typedef struct {
	char instance_name[30];
	esi_status_e status;
} t_esi_status_response;

static const int ESI_STATUS_RESPONSE_SIZE = 31 + 4;

typedef struct {
	operation_type_e operation_type;
	char key[40];
} t_coordinator_operation_request;

static const int COORDINATOR_OPERATION_REQUEST_SIZE = 4 + 41;

typedef struct {
	instance_status_e status;
	int payload_size;
} t_instance_response;

static const int INSTANCE_RESPONSE_SIZE = 4 + 4;

typedef struct {
	int entry_size;
	int number_of_entries;
} t_instance_init_values;

static const int INSTANCE_INIT_VALUES_SIZE = 4 + 4;

typedef struct {
	coordinator_operation_type_e coordinator_operation_type;
} t_coordinator_operation_header;

static const int COORDINATOR_OPERATION_HEADER_SIZE = 4;


typedef struct{
	int payload_valor_size;
	char nombre_intancia_actual[40];
	char nombre_intancia_posible[40];
} status_response_from_coordinator;

static const int STATUS_RESPONSE_FROM_COORDINATOR = 4 + 41 + 41;

// Connection Messages
int connect_to_server(char *ip, int port, t_log *logger);
bool perform_connection_handshake(int server_socket, char* instance_name,
		instance_type_e instance_type, t_log *logger);
bool send_connection_header(int server_socket, char* instance_name,
		instance_type_e instance_type, t_log *logger);
bool wait_for_acknowledge(int server_socket, t_log *logger);

// Serialization
void* serialize_connection_header(t_connection_header *header);
t_connection_header* deserialize_connection_header(void* buffer);

void* serialize_ack_message(t_ack_message *ack_message);
t_ack_message* deserialize_ack_message(void* buffer);

void* serialize_operation_request(t_operation_request *request);
t_operation_request* deserialize_operation_request(void* buffer);

void* serialize_operation_response(t_operation_response *response);
t_operation_response* deserialize_operation_response(void* buffer);

void* serialize_planner_execute_request(t_planner_execute_request *request);
t_planner_execute_request* deserialize_planner_execute_request(void *buffer);

void* serialize_esi_status_response(t_esi_status_response *response);
t_esi_status_response* deserialize_esi_status_response(void *buffer);

void* serialize_coordinator_operation_request(
		t_coordinator_operation_request *request);
t_coordinator_operation_request* deserialize_coordinator_operation_request(
		void *buffer);

void* serialize_instance_response(t_instance_response *response);
t_instance_response* deserialize_instance_response(void *buffer);

void* serialize_instance_init_values(t_instance_init_values *response);
t_instance_init_values* deserialize_instance_init_values(void *buffer);

void* serialize_coordinator_operation_header(
		t_coordinator_operation_header *header);
t_coordinator_operation_header* deserialize_coordinator_operation_header(
		void* buffer);

void* serialize_status_response_from_coordinator(status_response_from_coordinator *response);
status_response_from_coordinator* derialize_status_response_from_coordinator(void *buffer);

#endif /* _PROTOCOLS_H_ */
