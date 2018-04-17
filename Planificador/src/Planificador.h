#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <signal.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include "libs/serialize.h"
#include "libs/tcpserver.h"

/*MACROS*/
#define PLANNER_CFG_FILE "planificador.config"

#ifndef PLANIFICADOR_SRC_PLANIFICADOR_H_
#define PLANIFICADOR_SRC_PLANIFICADOR_H_

t_log *console_log;

enum AlgortimoPlanificacion {
	SJF_CD = 1, SJF_SD = 2, HRRN = 3
};

char* instance_name = NULL;
char *coordinator_ip = NULL;
int coordinator_port = 0;
int coordinator_socket = 0;
int initial_estimation = 0;
int planification_algorithm = 0;
int port_to_listen = 0;
int max_clients = 30;
int connection_queue_size = 10;
char** initial_blocked_keys = NULL;

tcp_server_t* server;

/*FUNCIONES*/
void exit_gracefully(int retVal);
void load_configuration();

// Funciones de la aplicacion del algoritmo
void applyPlaningAlgorithm();
void getNextESI();
void moveOutCurrentESI();
void continueExecutingESI();

//Funciones para los recursos
bool isResourceAvailable();
void lockResource();
void unlockResource();

//Funciones para la administracion de los ESI
void lockESI();
void unlockESI();
void finishESI();

//Comunicacion con el coordinador
void sendLockResourceOperationResult(bool p_result);
void sendUnlockResourceOperationResult(bool p_result);

// TCP Server handlers
void on_server_accept(tcp_server_t* server, int client_socket, int socket_id);
void on_server_read(tcp_server_t* server, int client_socket, int socket_id);
void on_server_command(tcp_server_t* server);
void before_tpc_server_cycle(tcp_server_t* server);

#endif /* PLANIFICADOR_SRC_PLANIFICADOR_H_ */
