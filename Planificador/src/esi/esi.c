#include "esi.h"

void inicializarListasEsi(){
	listaEsiListos = list_create();
	listaEsiBloqueados = list_create();
	listaEsiTerminados = list_create();
}

ESI_STRUCT * nuevoESI(int p_id, int p_client_socket, int p_socket_id) {
	ESI_STRUCT * nuevoEsi = malloc(sizeof(ESI_STRUCT));
	nuevoEsi->id = p_id;
	nuevoEsi->client_socket = p_client_socket;
	nuevoEsi->socket_id = p_socket_id;
	nuevoEsi->estado = LISTO;
	return nuevoEsi;
}

ESI_STRUCT * clonarEsi(ESI_STRUCT *esi) {
	ESI_STRUCT * nuevoEsi = malloc(sizeof(ESI_STRUCT));
	nuevoEsi->id = esi->id;
	nuevoEsi->client_socket = esi->socket_id;
	nuevoEsi->socket_id = esi->socket_id;
	nuevoEsi->estado = esi->estado;
	return nuevoEsi;
}


void agregarNuevoEsi(ESI_STRUCT * esi){
	list_add(listaEsiListos, esi);
}

void terminarEsiActual(){
	esiEjecutando->estado = TERMINADO;
	list_add(listaEsiTerminados, clonarEsi(esiEjecutando));
	free(esiEjecutando);
	esiEjecutando = NULL;
}



void listarEsi(t_log * console_log) {
	log_error(console_log, "Consola: Listar ESIs");
	printf("*******************************************\n");
	printf("ESI\t| ESTADO\n");
	printf("-------------------------------------------\n");

	printf("-------------------------------------------\n");
	printf("\n");
}

/**
 * Encola un ESI en la lista de bloqueados
 */
void bloquearEsi() {
	esiEjecutando->estado = BLOQUEADO;
	list_add(listaEsiBloqueados, clonarEsi(esiEjecutando));
	free(esiEjecutando);
	esiEjecutando = NULL;
}

/**
 * Desencola un ESI de la lista de bloqueados
 */
void desbloquearEsi(ESI_STRUCT * esi) {
	int i = 0, tamanioLista = list_size(listaEsiBloqueados);
	ESI_STRUCT * esiAux = NULL;

	for(i = 0; i < tamanioLista; i++)
	{
		esiAux = list_get(listaEsiBloqueados, i);
		if(esiAux != NULL && sonIguales(esi, esiAux))
		{
			list_remove(listaEsiBloqueados, i);
			list_add(listaEsiListos, esiAux);
			return;
		}
	}
}

bool sonIguales(ESI_STRUCT * esi1, ESI_STRUCT * esi2){
	return esi1->id == esi2->id &&
			esi1->socket_id == esi2->socket_id &&
			esi1->client_socket == esi2->client_socket;
}

void liberarEsi(ESI_STRUCT * esi) {
	free(esi);
}

void liberarRecursosEsi() {
	list_destroy_and_destroy_elements(listaEsiListos, (void*)liberarEsi);
	list_destroy_and_destroy_elements(listaEsiBloqueados, (void*)liberarEsi);
	list_destroy_and_destroy_elements(listaEsiTerminados, (void*)liberarEsi);
}
