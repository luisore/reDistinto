#ifndef SRC_COMMONS_COMMONS_H_
#define SRC_COMMONS_COMMONS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <commons/string.h>

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
	ESI_BLOCKED_INFO informacionDeBloqueo;
	int tiempoEspera;
} ESI_STRUCT;

typedef struct {
	char * nombre_recurso;
	ESI_STRUCT * esi_bloqueante;
	RECURSO_ESTADO estado;
} RECURSO;

// El log lo hago comun para todos los archivos
t_log *console_log;

int create_log();

#endif /* SRC_COMMONS_COMMONS_H_ */
