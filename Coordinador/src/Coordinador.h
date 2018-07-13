#include <commons/config.h>
#include <commons/log.h>
#include <stdio.h> //printf
#include <commons/collections/list.h>
#include "libs/tcpserver.h"
#include "libs/protocols.h"
#include <pthread.h>

#ifndef SRC_COORDINADOR_H_
#define SRC_COORDINADOR_H_

/*MACROS*/
#define PATH_FILE_NAME "coordinador.config"

/*VARIABLES GLOBALES*/
t_log * coordinador_log;
t_config *config;
t_list* connected_clients;
t_list* connected_instances;
int instancia_actual=0;

tcp_server_t* server;
tcp_server_t* server_planner_console;

char* instance_name = NULL;
char *planificador_ip = NULL;
int planificador_port = 0;
int planificador_socket = 0;
char** initial_blocked_keys = NULL;

t_list * lista_instancias;

/*ESTRUCTURAS*/

t_dictionary* key_instance_dictionary;



/*SEMAFOROS - SINCRONIZACION*/

pthread_t thread_planner_console;
pthread_t thread_principal;

pthread_mutex_t mutex_planner_console;
pthread_mutex_t mutex_principal;
pthread_mutex_t mutex_all;

typedef enum {
	LSU = 1, EL = 2, KE = 3
} dist_algo_e;

typedef struct {
	char * NOMBRE_INSTANCIA;
	int PUERTO_ESCUCHA_CONEXIONES;
	int CANTIDAD_MAXIMA_CLIENTES;
	int TAMANIO_COLA_CONEXIONES;
	dist_algo_e ALGORITMO_DISTRIBUCION;
	int CANTIDAD_ENTRADAS;
	int TAMANIO_ENTRADA_BYTES;
	int RETARDO_MS;
	int PUERTO_ESCUCHA_CONEXION_CONSOLA;
} coordinador_setup;

coordinador_setup coordinador_config;

typedef struct {
	char instance_name[30];
	instance_type_e instance_type;
	int socket_id;
	int socket_reference;
} t_connected_client;


typedef struct{
	t_connected_client * instance;
	bool isConnected;
	int storage;
} t_dictionary_instance_struct;



/* PRINCIPAL FUNCTIONS*/
void coordinate_planner_console();
void coordinate_principal_process();

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
t_connected_client* find_connected_client_by_type(instance_type_e instance_type);
t_connected_client* select_instancia();

bool send_operation_to_instance(t_connected_client * instance);


t_instance_response *  send_get_operation( char * key ,t_connected_client *instance);
bool send_store_operation(t_operation_request* esi_request, operation_type_e t, t_connected_client *instance);
bool send_set_operation(t_operation_request* esi_request, operation_type_e t, t_connected_client *instance , char * payload_value);

t_instance_response * receive_response_from_instance(t_connected_client * instance);

// Must return value char * . For now its ok.
char *  receive_value_from_instance(t_connected_client * instance , int payload_size);


/*ALGORITHIMS*/

t_connected_client* select_intance_LSU();
t_connected_client* select_intance_EL(char * key);
t_connected_client* select_intance_KE(bool flag_simulation);


#endif /* SRC_COORDINADOR_H_ */
