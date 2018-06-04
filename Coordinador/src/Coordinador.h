#include <commons/config.h>
#include <commons/log.h>
#include <stdio.h> //printf
#include <commons/collections/list.h>
#include "libs/tcpserver.h"
#include "libs/protocols.h"

#ifndef SRC_COORDINADOR_H_
#define SRC_COORDINADOR_H_

/*MACROS*/
#define PATH_FILE_NAME "coordinador.config"

/*VARIABLES GLOBALES*/
t_log * coordinador_log;
t_config *config;
t_list* connected_clients;

tcp_server_t* server;
char* instance_name = NULL;
char *planificador_ip = NULL;
int planificador_port = 0;
int planificador_socket = 0;
char** initial_blocked_keys = NULL;
t_list * lista_instancias;
/*ESTRUCTURAS*/



enum AlgortimoDistribucion {
	LSU = 1, EL = 2, KE = 3
};

struct {
	char * NOMBRE_INSTANCIA;
	int PUERTO_ESCUCHA_CONEXIONES;
	int CANTIDAD_MAXIMA_CLIENTES;
	int TAMANIO_COLA_CONEXIONES;
	enum AlgortimoDistribucion ALGORITMO_DISTRIBUCION;
	int CANTIDAD_ENTRADAS;
	int TAMANIO_ENTRADA_BYTES;
	int RETARDO_MS;
} coordinador_setup;

typedef struct {
	char instance_name[30];
	instance_type_e instance_type;
	int socket_id;
} t_connected_client;


/*FUNCIONES GENERALES*/

void print_header();
void create_log();
void loadConfig();
void log_inicial_consola();
void print_goodbye();
void exit_program(int);

void liberar_memoria();
void enviar_respuesta_esi(int esi_socket, int socket_id);
void destroy_connected_client(t_connected_client* connected_client);
t_connected_client* find_connected_client(int socket_id);
void send_response_to_esi(int esi_socket, t_connected_client* client, operation_result_e op_result);
void send_message_instance(t_connection_header *connection_header, int client_socket, int socket_id);
void send_message_clients(t_connection_header *connection_header, int client_socket, int socket_id);

#endif /* SRC_COORDINADOR_H_ */
