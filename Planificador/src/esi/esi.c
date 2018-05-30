#include "esi.h"

void inicializarListasEsi(){
	listaEsis = list_create();
	listaEsiListos = list_create();
	listaEsiBloqueados = list_create();
	listaEsiTerminados = list_create();
	listaRecursos = list_create();
	listaEsiNuevos = list_create();
}

ESI_STRUCT * nuevoESI(int p_id, int p_client_socket, int p_socket_id) {
	ESI_STRUCT * nuevoEsi = malloc(sizeof(ESI_STRUCT));
	nuevoEsi->id = p_id;
	nuevoEsi->client_socket = p_client_socket;
	nuevoEsi->socket_id = p_socket_id;
	nuevoEsi->estado = ESI_LISTO;
	nuevoEsi->tiempoEspera = 0;
	return nuevoEsi;
}

ESI_STRUCT * clonarEsi(ESI_STRUCT *esi) {
	ESI_STRUCT * nuevoEsi = esi;
	return nuevoEsi;
}


void agregarNuevoEsi(ESI_STRUCT * esi){
	list_add(listaEsiNuevos, esi);
}

void terminarEsiActual(){
	esiEjecutando->estado = ESI_TERMINADO;
	// TODO: no me gusta esto. Cambiar. Investigar punteros
	list_add(listaEsiTerminados, clonarEsi(esiEjecutando));
	esiEjecutando = NULL;
}

/**
 * Encola un ESI en la lista de bloqueados
 */
void bloquearEsi(char * recursoEsperado) {

	// Creo la informacion de bloqueo
	ESI_BLOCKED_INFO* infoBloqueo = malloc(sizeof(ESI_BLOCKED_INFO));
	infoBloqueo->recursoNecesitado = "";
	strcpy(infoBloqueo->recursoNecesitado,recursoEsperado);
	infoBloqueo->unidadesDeTiempoBloqueado = 0;

	esiEjecutando->estado = ESI_BLOQUEADO;
	esiEjecutando->informacionDeBloqueo = infoBloqueo;

	// Encolo el esi en la lista de bloqueados
	list_add(listaEsiBloqueados, clonarEsi(esiEjecutando));

	// Libero el esi actual
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
			esiAux->estado = ESI_LISTO;
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

int bloquearRecurso(char * p_recurso){
	int i = 0, tamanioLista = 0, encontrado = 0;

	if(listaRecursos == NULL)
		listaRecursos = list_create();

	tamanioLista = list_size(listaRecursos);

	for(i = 0; i < tamanioLista; i++){
		RECURSO* r = list_get(listaRecursos, i);

		if(string_equals_ignore_case(r->nombre_recurso, p_recurso))
		{
			r->estado = RECURSO_BLOQUEADO;
			r->esi_bloqueante = esiEjecutando;

			encontrado = 1;
			break;
		}
	}

	if(encontrado == 0)
	{
		RECURSO * r = malloc(sizeof(RECURSO));
		r->esi_bloqueante = esiEjecutando;
		r->nombre_recurso = p_recurso;
		r->estado = RECURSO_BLOQUEADO;

		list_add(listaRecursos, r);

		return 0;
	}
	return -1;
}

int liberarRecurso(char * p_recurso){
	int i = 0, tamanioLista = list_size(listaRecursos);

	for(i = 0; i < tamanioLista; i++){
		RECURSO* r = list_get(listaRecursos, i);

		if(string_equals_ignore_case(r->nombre_recurso, p_recurso))
		{
			r->estado = RECURSO_LIBRE;
			r->esi_bloqueante = NULL;

			return 0;
		}
	}

	return -1;
}

RECURSO_ESTADO estadoRecurso(char * p_recurso){
	int i = 0, tamanioLista = list_size(listaRecursos);

	for(i = 0; i < tamanioLista; i++) {
		RECURSO* r = list_get(listaRecursos, i);

		if(string_equals_ignore_case(r->nombre_recurso, p_recurso))
		{
			return r->estado;
		}
	}

	return RECURSO_UNK;
}

void liberarEsi(ESI_STRUCT * esi) {
	free(esi);
}

void liberarTablaClaves(RECURSO * p_recurso){
	free(p_recurso->nombre_recurso);
	free(p_recurso);
}

void liberarRecursosEsi() {
	list_destroy_and_destroy_elements(listaEsiListos, (void*)liberarEsi);
	list_destroy_and_destroy_elements(listaEsiBloqueados, (void*)liberarEsi);
	list_destroy_and_destroy_elements(listaEsiTerminados, (void*)liberarEsi);
	list_destroy_and_destroy_elements(listaEsis, (void*)liberarEsi);
	list_destroy_and_destroy_elements(listaRecursos, (void*)liberarTablaClaves);
	list_destroy_and_destroy_elements(listaEsiNuevos, (void*)liberarEsi);
}
