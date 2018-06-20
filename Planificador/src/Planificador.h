#ifndef PLANIFICADOR_SRC_PLANIFICADOR_H_
#define PLANIFICADOR_SRC_PLANIFICADOR_H_

#include <signal.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include <library/serialize.h>
#include <library/tcpserver.h>
#include <library/protocols.h>

#include "configuracion/configuracion.h"
#include "esi/esi.h"
#include "consola/consola.h"
#include "planificacion/planificacion.h"
#include "coordinador/Coordinador.h"

#define PLANNER_CFG_FILE "planificador.config"

int esi_id = 0;

tcp_server_t* server;

pthread_t hiloConsola;
pthread_t hiloPrincipal;
pthread_t hiloPlanificacion;

/*FUNCIONES*/
void imprimirCabecera();
int inicializar();
void liberarRecursos(int tipoSalida);
void imprimirFin();
void exit_gracefully(int retVal);

int generarId();

//Hilos
void escucharConsola();
void iniciarPlanificador();
void ejecutarPlanificacion();

void aplicar_algoritmo_planificacion();
int esperarEstadoDelEsi(int esi_socket, int socket_id);
void ejecutarSiguienteESI(int esi_socket, int socket_id);

// TCP Server handlers
void create_tcp_server();
void on_server_accept(tcp_server_t* server, int client_socket, int socket_id);
void on_server_read(tcp_server_t* server, int client_socket, int socket_id);
void on_server_command(tcp_server_t* server);
void before_tpc_server_cycle(tcp_server_t* server);

#endif /* PLANIFICADOR_SRC_PLANIFICADOR_H_ */
