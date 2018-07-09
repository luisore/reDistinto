#ifndef SRC_COMMONS_COMMONS_H_
#define SRC_COMMONS_COMMONS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <semaphore.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <sys/socket.h>
#include <commons/string.h>
#include <pthread.h>

typedef enum  {
	ESI_LISTO,
	ESI_EJECUTANDO,
	ESI_BLOQUEADO,
	ESI_TERMINADO
} ESI_STATUS;

typedef struct {
	char * recursoNecesitado;
	int unidadesDeTiempoBloqueado;
} ESI_BLOCKED_INFO;

typedef enum {
	RECURSO_UNK,
	RECURSO_LIBRE,
	RECURSO_BLOQUEADO
} RECURSO_ESTADO;

typedef struct {
	int id;
	int client_socket;
	int socket_id;
	ESI_STATUS estado;
	ESI_BLOCKED_INFO* informacionDeBloqueo;
	int tiempoEspera;
	int tiempoEstimado;
	int tiempoRafagaActual;
} ESI_STRUCT;

typedef struct {
	char * nombre_recurso;
	ESI_STRUCT * esi_bloqueante;
	RECURSO_ESTADO estado;
} RECURSO;

// El log lo hago comun para todos los archivos
t_log *console_log;

int coordinator_socket_console ;

pthread_mutex_t mutexConsola;
pthread_mutex_t mutexLog;
pthread_mutex_t mutexPrincipal;

sem_t sem_esis;

int create_log();

void info_log(char * message);
void info_log_param1(char * message, void * param1);
void info_log_param2(char * message, void * param1, void * param2);
void error_log(char * message);
void error_log_param1(char * message, void * param1);
void error_log_param2(char * message, void * param1, void * param2);

#endif /* SRC_COMMONS_COMMONS_H_ */
