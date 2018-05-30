/*
 * protocols.h
 *
 *  Created on: 18 abr. 2018
 *      Author: avinocur
 */

#ifndef _PROTOCOLS_H_
#define _PROTOCOLS_H_
#include "serialize.h"
#include <stdbool.h>
#include <commons/log.h>

typedef enum { ESI = 0, COORDINATOR = 1, PLANNER = 2, REDIS_INSTANCE = 3 } instance_type_e;

typedef enum  { GET = 0, SET = 1, STORE = 2 } operation_type_e;

typedef enum { ESI_IDLE = 0, ESI_BLOCKED = 1, ESI_FINISHED = 2 } esi_status_e;
typedef enum { STATUS = 0 } instancia_status_e;

/*
 * Result of the operation performed by the Coordinator. It can be:
 * OP_SUCCESS = The requested operation was performed.
 * OP_ERROR   = There was an error with the requested key.
 * OP_BLOCKED = The requested key is blocked by another ESI.
 */
typedef enum { OP_SUCCESS = 0, OP_ERROR = 1, OP_BLOCKED = 2 } operation_result_e;

/*
 * Header used in every connection between processes.
 * Either when connecting for the first time or before sending any message.
 */
typedef struct {
	char instance_name[30];
	instance_type_e instance_type;
} t_connection_header;

static const int CONNECTION_HEADER_SIZE = 31 + 4;


// Cada proceso debe castear el response segun sea necesario  y segun cada situacion.

typedef struct {
	instance_type_e instance_type;
	char response[60];

} t_response_process;

static const int CONNECTION_PACKAGE_SIZE = 4 + 61;




typedef struct {
	char instance_name[30];
} t_ack_message;

static const int ACK_MESSAGE_SIZE = 31;

static const int ESI_INSTRUCTION_REQUEST_SIZE = 31 + 4;

/*
 * ESI Operation to send to the Coordinator
 */
typedef struct {
	operation_type_e operation_type;
	char key[40];
	unsigned int payload_size;
} t_esi_operation_request;

static const int ESI_OPERATION_REQUEST_SIZE = 4 + 41 + 4;

/*
 * Coordinator response with the operation result
 */
typedef struct {
	operation_result_e operation_result;
} t_coordinator_operation_response;

static const int COORD_OPERATION_RESPONSE_SIZE = 4;


/*
 * Planner request to execute next instruction
 */
typedef struct {
	char planner_name[30];
} t_planner_request;


typedef struct {
	char coordinador_name[30];
} t_coordinador_request;


typedef struct {
	char coordinador_name[30];
	int TAMANIO_ENTRADA_BYTES;
	int CANTIDAD_ENTRADAS;
} t_coordinador_request_instancia;

typedef struct {
	int id_socket;
	int id_instancia;
	int tamanio_entrada_bytes;
	int vantidad_entradas;
} t_instancia;

static const int PLANNER_REQUEST_SIZE = 31;
static const int INSTANCIA_REQUEST_SIZE = 31+4+4;

/*
 * ESI response with the current status
 */
typedef struct {
	char instance_name[30];
	esi_status_e status;
} t_esi_status_response;

static const int ESI_STATUS_RESPONSE_SIZE = 31 + 4;

// Connection Messages

// Serialization
t_connection_header* deserialize_connection_header(void* buffer);

t_response_process * deserialize_abstract_response (void *buffer);


void* serialize_ack_message(t_ack_message *ack_message);
t_ack_message* deserialize_ack_message(void* buffer);

void* serialize_esi_status_response(t_esi_status_response *response);
t_esi_status_response* deserialize_esi_status_response(void *buffer);
void* serialize_coordinador_request(t_coordinador_request *request);
int serialize_data(void *object, int nBytes, void **buffer, int *lastIndex);

#endif /* _PROTOCOLS_H_ */
