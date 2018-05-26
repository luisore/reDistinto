#ifndef PLANIFICADOR_SRC_PLANIFICADOR_H_
#define PLANIFICADOR_SRC_PLANIFICADOR_H_

#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "libs/serialize.h"
#include "libs/tcpserver.h"
#include "libs/protocols.h"

#include "configuracion/configuracion.h"
#include "esi/esi.h"
#include "consola/consola.h"
#include "planificacion/planificacion.h"

#define PLANNER_CFG_FILE "planificador.config"

int coordinator_socket = 0;
int esi_id = 0;

tcp_server_t* server;

pthread_t hiloConsola;
pthread_t hiloPrincipal;
pthread_mutex_t mutexConsola;
pthread_mutex_t mutexPrincipal;

/*FUNCIONES*/
void exit_gracefully(int retVal);
void liberarRecursos(int tipoSalida);
int inicializar();
void print_header();
int generarId();

void escucharConsola();
void iniciarPlanificador();

// Funciones de la aplicacion del algoritmo
void applyPlaningAlgorithm();
void getNextESI();
void moveOutCurrentESI();
void continueExecutingESI();

//Comunicacion con el coordinador
void conectarseConCoordinador();
void sendLockResourceOperationResult(bool p_result);
void sendUnlockResourceOperationResult(bool p_result);

// TCP Server handlers
void create_tcp_server();
void on_server_accept(tcp_server_t* server, int client_socket, int socket_id);
void on_server_read(tcp_server_t* server, int client_socket, int socket_id);
void on_server_command(tcp_server_t* server);
void before_tpc_server_cycle(tcp_server_t* server);

#endif /* PLANIFICADOR_SRC_PLANIFICADOR_H_ */
