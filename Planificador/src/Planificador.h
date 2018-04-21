#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <signal.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include "libs/socketCommons.h"
#include "libs/serialize.h"
#include "libs/tcpserver.h"

/*MACROS*/
#define PLANNER_CFG_FILE "planificador.config"

#ifndef PLANIFICADOR_SRC_PLANIFICADOR_H_
#define PLANIFICADOR_SRC_PLANIFICADOR_H_

t_log *console_log;

struct ESI_STRUCT{
	int id; // Por si necesitamos llevar un identificacion interna
	int client_socket;
	int socket_id;
};

enum AlgortimoPlanificacion {
	SJF_CD = 1, SJF_SD = 2, HRRN = 3
};

int coordinator_socket = 0;

struct {
	char* NOMBRE_INSTANCIA;
	char* IP_COORDINADOR;
	int PUERTO_COORDINADOR;
	enum AlgortimoPlanificacion ALGORITMO_PLANIFICACION;
	int ESTIMACION_INICIAL;
	int PUERTO_ESCUCHA_CONEXIONES;
	char** CLAVES_INICIALMENTE_BLOQUEADAS;
	int CANTIDAD_MAXIMA_CLIENTES;
	char** TAMANIO_COLA_CONEXIONES;
} planificador_setup;

tcp_server_t* server;

t_queue* listaEsiListos;
t_queue* listaEsiBloqueados;
t_queue* listaEsiTerminados;

struct ESI_STRUCT* esiEjecutando;

/*FUNCIONES*/
void exit_gracefully(int retVal);
int load_configuration(char* archivoConfiguracion);
int inicializar();

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
