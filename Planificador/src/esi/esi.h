#ifndef SRC_ESI_ESI_H_
#define SRC_ESI_ESI_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/string.h>

enum ESI_STATUS {
	LISTO,
	EJECUTANDO,
	BLOQUEADO,
	TERMINADO
};

typedef struct {
	int id; // Por si necesitamos llevar un identificacion interna
	int client_socket;
	int socket_id;
	enum ESI_STATUS estado;
} ESI_STRUCT;


t_list* listaEsiListos;
t_list* listaEsiBloqueados;
t_list* listaEsiTerminados;

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

#endif /* SRC_ESI_ESI_H_ */
