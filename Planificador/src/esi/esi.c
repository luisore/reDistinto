#include "esi.h"

struct ESI_STRUCT * nuevoESI(int p_id, int p_client_socket, int p_socket_id) {
	struct ESI_STRUCT * nuevoEsi = malloc(sizeof(struct ESI_STRUCT));
	nuevoEsi->id = p_id;
	nuevoEsi->client_socket = p_client_socket;
	nuevoEsi->socket_id = p_socket_id;
	nuevoEsi->estado = LISTO;
	return nuevoEsi;
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
// TODO: falta pasar el ESI por parametro
void lockESI() {
	//TODO: implementacion pendiente
}

/**
 * Desencola un ESI de la lista de bloqueados
 */
// TODO: falta pasar el ESI por parametro
void unlockESI() {
	//TODO: implementacion pendiente
}

/**
 * Finaliza un ESI
 */
// TODO: falta pasar el ESI por parametro
void finishESI() {
	//TODO: implementacion pendiente
}

void liberarRecursosEsi() {
}
