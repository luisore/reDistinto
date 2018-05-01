#ifndef SRC_ESI_ESI_H_
#define SRC_ESI_ESI_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/string.h>

typedef enum  {
	ESI_LISTO,
	ESI_EJECUTANDO,
	ESI_BLOQUEADO,
	ESI_TERMINADO
} ESI_STATUS;

typedef enum {
	RECURSO_UNK,
	RECURSO_LIBRE,
	RECURSO_BLOQUEADO
} RECURSO_ESTADO;

typedef struct {
	int id; // Por si necesitamos llevar un identificacion interna
	int client_socket;
	int socket_id;
	ESI_STATUS estado;
} ESI_STRUCT;

typedef struct {
	char * nombre_recurso;
	ESI_STRUCT * esi_bloqueante;
	RECURSO_ESTADO estado;
} RECURSO;


t_list* listaEsiListos;
t_list* listaEsiBloqueados;
t_list* listaEsiTerminados;

t_list* listaRecursos;

ESI_STRUCT* esiEjecutando;

void inicializarListasEsi();
ESI_STRUCT * nuevoESI(int p_id, int p_client_socket, int p_socket_id);
ESI_STRUCT * clonarEsi(ESI_STRUCT *esi);

//Funciones para la consola
void listarEsi(t_log * console_log);

void agregarNuevoEsi(ESI_STRUCT * esi);
void terminarEsiActual();

//Funciones para la administracion de los ESI
void bloquearEsi();
void desbloquearEsi();
bool sonIguales(ESI_STRUCT * esi1, ESI_STRUCT * esi2);

void liberarRecursosEsi();
void liberarEsi(ESI_STRUCT * esi);


// Recursos
int bloquearRecurso(char * p_recurso);
int liberarRecurso(char * p_recurso);
RECURSO_ESTADO estadoRecurso(char * p_recurso);

#endif /* SRC_ESI_ESI_H_ */
