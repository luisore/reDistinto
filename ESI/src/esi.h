#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <signal.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/queue.h>
#include "libs/serialize.h"
#include "libs/tcpserver.h"
#include "libs/protocols.h"

#ifndef ESI_H_
#define ESI_H_

#define ESI_CFG_FILE "esi.config"
#define PROGRAM_LINE_MAX_LENGTH 150
#define MAX_KEY_LENGTH 40

/* Global Variables */

t_log *esi_log = NULL;

// Configuration of the instance
char *instance_name = NULL;
char *coordinator_ip = NULL;
int coordinator_port = 0;
char *planner_ip = NULL;
int planner_port = 0;

// Sockets to communicate with the Coordinator and Planner
int coordinator_socket;
int planner_socket;


typedef struct {
	operation_type_e operation_type;
	char key[40];
	unsigned int value_size;
	char *value;
} t_program_instruction;

// Free all the resources used by the ESI
void exit_gracefully(int retVal);

// Load the configuration from the ESI_CFG_FILE
void load_config();

// Print hello and goodbye messages on the console
void print_header();
void print_goodbye();

// Create the logger on esi.log
void create_log();

// Connect with the Coordinator and Planner.
void connect_with_coordinator();
void connect_with_planner();


/*
 * Sends the ESI status to the Planner.
 * This happens after coordinating the instruction with the Coordinator.
 * The ESI will expect a new instruction from the Planner when signaling
 * ESI_IDLE or ESI_BLOCKED status to resume execution, and exit and close
 * the connection socket after signaling the ESI_FINISHED status.
 */
bool send_status_to_planner(esi_status_e esi_status);

/*
 * Parse the program instructions from the file received as a parameter.
 * File correctness is not checked. All instructions are presumed to be executed in
 * the right order.
 * Lines on the file can be one of three possible instructions:
 *    SET key value
 *    GET key
 *    STORE key
 * keys must be strings of up to 40 characters.
 * If an invalid key is supplied, the ESI will not execute.
 */
t_queue* parse_program_instructions(char *filename);

// Wait for the Planner to signal that the ESI can execute the next instruction.
bool wait_for_planner_signal();

// Destroys the program instruction, which must be of type t_program_instruction*
void destroy_program_instruction(void *instruction);

/*
 * Coordinates the execution of the instruction with the Coordinator.
 * The result can be either:
 *     OP_SUCCESS: The resource (key) has been accessed and assigned to this ESI.
 *     OP_ERROR: The resource (key) was not found.
 *     OP_BLOCKED: The resource (key) is blocked by the Planner or assigned to another ESI.
 */
operation_result_e coordinate_operation(t_program_instruction *instruction);

/*
 * Executes the program instructions one by one, following the sequence:
 *   1. Wait for the Planner to signal that the next instruction can be executed
 *   2. Communicate with the Coordinator to execute the operation (if possible)
 *   3. Send the ESI status to the Planner:
 *   	 TODO: DEFINE WHETHER AN ESI_NEW STATUS IS NECESSARY
 *   	 TODO: MAYBE ESY_READY IS MORE DESCRIPTIVE THAN ESI_IDLE
 *       ESI_IDLE: The ESI is ready to execute the next instruction.
 *       ESI_BLOCKED: The ESI is blocked waiting for a resource that is not available.
 *       ESI_FINISHED: The ESI finished the execution.
 *   4. If the ESI did not finish, repeat 1.
 */
void execute_program();


#endif /* ESI_H_ */
