#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <signal.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <library/serialize.h>
#include <library/protocols.h>
#include <stdbool.h>
#include "redis.h"

#ifndef SRC_INSTANCIA_H_
#define SRC_INSTANCIA_H_

t_log *console_log;
t_config *config;

int coordinator_socket;

/*MACROS*/
#define INSTANCE_CFG_FILE "instancia.config"

/*ESTRUCTURAS*/
typedef enum {
	CIRC = 1, LRU = 2, BSU = 3
} replacement_algo_e;

typedef struct {
	operation_type_e operation_type;
	char key[40];
	unsigned int value_size;
	char *value;
} t_operation;

typedef struct {
	char* IP_COORDINADOR;
	int PUERTO_COORDINADOR;
	replacement_algo_e ALGORITMO_REEMPLAZO;
	char* PUNTO_MONTAJE;
	char* NOMBRE_INSTANCIA;
	int INTERVALO_DUMP_SEGs;
} t_instance_setup;

t_instance_setup instance_setup;

int entry_size;
int number_of_entries;
int storage_size;

t_redis* redis;



/*FUNCIONES GENERALES*/

void print_header();
void create_log();
void loadConfig();
void log_inicial_consola();

void initialize_instance();
void load_dump_files();

void connect_with_coordinator();

void print_goodbye();
void exit_program(int);

/*FUNCIONES MEMORIA*/
void lectura_archivo();

void instance_setup_destroy(t_instance_setup* instance_setup);


#endif /* SRC_INSTANCIA_H_ */
