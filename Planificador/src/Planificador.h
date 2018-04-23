#ifndef PLANIFICADOR_SRC_PLANIFICADOR_H_
#define PLANIFICADOR_SRC_PLANIFICADOR_H_

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include "libs/serialize.h"
#include "libs/tcpserver.h"
#include <sys/socket.h> // Para crear sockets, enviar, recibir, etc
#include <netdb.h> // Para getaddrinfo
#include <unistd.h> // Para close
#include "libs/protocols.h"
#include "configuracion/configuracion.h"

/*MACROS*/
#define PLANNER_CFG_FILE "planificador.config"

struct ESI_STRUCT{
	int id; // Por si necesitamos llevar un identificacion interna
	int client_socket;
	int socket_id;
};

t_log *console_log;

int coordinator_socket = 0;

tcp_server_t* server;

t_queue* listaEsiListos;
t_queue* listaEsiBloqueados;
t_queue* listaEsiTerminados;

struct ESI_STRUCT* esiEjecutando;

/*FUNCIONES*/
void exit_gracefully(int retVal);
void liberarRecursos(int tipoSalida);
void create_log();

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
