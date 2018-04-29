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
	BLOQUEADO
};

struct ESI_STRUCT{
	int id; // Por si necesitamos llevar un identificacion interna
	int client_socket;
	int socket_id;
	enum ESI_STATUS estado;
};


t_queue* listaEsiListos;
t_queue* listaEsiBloqueados;
t_queue* listaEsiTerminados;

struct ESI_STRUCT* esiEjecutando;


struct ESI_STRUCT * nuevoESI(int p_id, int p_client_socket, int p_socket_id);

//Funciones para la consola
void listarEsi(t_log * console_log);

//Funciones para la administracion de los ESI
void lockESI();
void unlockESI();
void finishESI();

void liberarRecursosEsi();

#endif /* SRC_ESI_ESI_H_ */
