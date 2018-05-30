#ifndef PLANIFICADOR_SRC_PLANIFICADOR_H_
#define PLANIFICADOR_SRC_PLANIFICADOR_H_

#include <signal.h>
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
pthread_t hiloPlanificacion;

/*FUNCIONES*/
void exit_gracefully(int retVal);
void liberarRecursos(int tipoSalida);
int inicializar();
void print_header();
int generarId();

void escucharConsola();
void iniciarPlanificador();
void ejecutarPlanificacion();

void aplicar_algoritmo_planificacion();

//Comunicacion con el coordinador
void conectarseConCoordinador();

// TCP Server handlers
void create_tcp_server();
void on_server_accept(tcp_server_t* server, int client_socket, int socket_id);
void on_server_read(tcp_server_t* server, int client_socket, int socket_id);
void on_server_command(tcp_server_t* server);
void before_tpc_server_cycle(tcp_server_t* server);

void send_get_status_to_esi(int esi_socket, int socket_id);
void send_execute_next_to_esi(int esi_socket, int socket_id);

#endif /* PLANIFICADOR_SRC_PLANIFICADOR_H_ */
