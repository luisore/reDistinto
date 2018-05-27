#ifndef SRC_ESI_ESI_H_
#define SRC_ESI_ESI_H_

#include "../commons/commons.h"

t_list* listaEsiNuevos;
t_list* listaEsiListos;
t_list* listaEsiBloqueados;
t_list* listaEsiTerminados;
t_list* listaEsis;

t_list* listaRecursos;

ESI_STRUCT* esiEjecutando;

void inicializarListasEsi();
ESI_STRUCT * nuevoESI(int p_id, int p_client_socket, int p_socket_id);
ESI_STRUCT * clonarEsi(ESI_STRUCT *esi);

//Funciones para la consola
void listarEsi();

void agregarNuevoEsi(ESI_STRUCT * esi);
void terminarEsiActual();

//Funciones para la administracion de los ESI
void bloquearEsi(char * recursoEsperado);
void desbloquearEsi();
bool sonIguales(ESI_STRUCT * esi1, ESI_STRUCT * esi2);

void liberarRecursosEsi();
void liberarEsi(ESI_STRUCT * esi);


// Recursos
int bloquearRecurso(char * p_recurso);
int liberarRecurso(char * p_recurso);
RECURSO_ESTADO estadoRecurso(char * p_recurso);

#endif /* SRC_ESI_ESI_H_ */
