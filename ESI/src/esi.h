#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <signal.h>
#include <commons/config.h>
#include <commons/log.h>
#include "libs/socketCommons.h"
#include "libs/serialize.h"
#include "libs/protocols.h"

#ifndef ESI_H_
#define ESI_H_

#define ESI_CFG_FILE "esi.config"

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


#endif /* ESI_H_ */
