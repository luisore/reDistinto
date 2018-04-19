/*
 * protocols.h
 *
 *  Created on: 18 abr. 2018
 *      Author: avinocur
 */

#ifndef _PROTOCOLS_H_
#define _PROTOCOLS_H_

enum instance_type_e { ESI = 0, COORDINATOR = 1, PLANNER = 2, REDIS_INSTANCE = 3 };

enum operation_type_e { GET = 0, SET = 1, STORE = 2 };

enum esi_status_e { ESI_IDLE = 0, ESI_BLOCKED = 1, ESI_FINISHED = 2 };

/*
 * Result of the operation performed by the Coordinator. It can be:
 * OP_SUCCESS = The requested operation was performed.
 * OP_ERROR   = There was an error with the requested key.
 * OP_BLOCKED = The requested key is blocked by another ESI.
 */
enum operation_result_e { OP_SUCCESS = 0, OP_ERROR = 1, OP_BLOCKED = 2 };

/*
 * Header used in every connection between processes.
 * Either when connecting for the first time or before sending any message.
 */
typedef struct {
	char instance_name[30];
	enum instance_type_e instance_type;
} t_connection_header;

typedef struct {
	char instance_name[30];
} t_ack_message;

/*
 * ESI Operation to send to the Coordinator
 */
typedef struct {
	enum operation_type_e operation_type;
	char key[40];
	unsigned int payload_size;
} t_esi_operation_request;

/*
 * Coordinator response with the operation result
 */
typedef struct {
	enum operation_result_e operation_result;
} t_coordinator_operation_response;


/*
 * Planner request to execute next instruction
 */
typedef struct {
	char planner_name[30];
} t_planner_request;

/*
 * ESI response with the current status
 */
typedef struct {
	enum esi_status_e status;
} t_esi_status_response;


#endif /* _PROTOCOLS_H_ */
