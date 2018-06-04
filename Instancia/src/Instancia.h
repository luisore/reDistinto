#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <signal.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include "libs/protocols.h"
#include "libs/serialize.h"
#include <stdbool.h>

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
	char* IP_COORDINADOR;
	int PUERTO_COORDINADOR;
	replacement_algo_e ALGORITMO_REEMPLAZO;
	char* PUNTO_MONTAJE;
	char* NOMBRE_INSTANCIA;
	int INTERVALO_DUMP_SEGs;
} t_instance_setup;

typedef struct {
	unsigned int size;
	int first_position;
} t_entry_data;

typedef struct {
	bool used;
	bool is_atomic;
	char key[40]; // the key that occupies this position
	int last_reference;
} t_memory_position;

typedef struct {
	operation_type_e operation_type;
	char key[40];
	unsigned int value_size;
	char *value;
} t_operation;

t_instance_setup instance_setup;

// Porcion de memoria asignada a las entradas
void* memory_region;

// Mapa de bools indicando si la porcion de memoria esta disponible o no
t_memory_position** occupied_memory_map;

// Diccionario de claves:
// key: Clave de la entrada
// value: t_entry_data
t_dictionary* key_dictionary;

// Configuracion recibida del Coordinador
int storage_size;
int entry_size;
int number_of_entries;

/*
 * Algoritmo actual de reemplazo de claves
 */
int (*perform_replacement_and_return_first_position)(unsigned int);
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

/*ALGORITMOS DE REEMPLAZO*/
int current_slot;

int replace_circular(unsigned int value_size);
//int lru_get_next_slot(); TODO
//int bsu_get_next_slot(); TODO

/*COMPACTACION Y DUMP*/

void compact();

void dump();

void entry_data_destroy(t_entry_data* entry_data);
void instance_setup_destroy(t_instance_setup* instance_setup);


#endif /* SRC_INSTANCIA_H_ */
