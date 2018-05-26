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

#ifndef SRC_INSTANCIA_H_
#define SRC_INSTANCIA_H_

t_log *console_log;
t_config *config;

int coordinator_socket;

/*MACROS*/
#define PATH_FILE_NAME "instancia.config"

/*ESTRUCTURAS*/
enum AlgortimoReemplazo {
	CIRC = 1, LRU = 2, BSU = 3
};

struct {
	char* IP_COORDINADOR;
	int PUERTO_COORDINADOR;
	enum AlgortimoReemplazo ALGORITMO_REEMPLAZO;
	char* PUNTO_MONTAJE;
	char* NOMBRE_INSTANCIA;
	int INTERVALO_DUMP_SEGs;
} instancia_setup;

// ESTRUCTURA PARA TABLA GENERAL

struct{
	int entradas_asociadas[100];
	int tamanio_key;
} estructura_key_tabla;

typedef struct {
	unsigned int tamanio;
	char *valor;
	int siguiente_instruccion;
} t_entrada;


int storage;
int tamanio_entradas;
int cantidad_entradas;

t_dictionary * tabla_entradas;
t_dictionary * tabla_claves;
t_list * lista_entradas;


/*FUNCIONES GENERALES*/

void print_header();
void create_log();
void loadConfig();
void log_inicial_consola();

void init_structs();
void load_dump_files();

void connect_with_coordinator();



void send_example();

void build_tabla_entradas();
void show_structs();

void print_goodbye();
void exit_program(int);

/*FUNCIONES MEMORIA*/
void lectura_archivo();
void liberar_memoria();


/*ALGORITMOS DE REEMPLAZO*/
void reemplazoCircular();
void reemplazoLeastRecentlyUsed();
void reemplazoBiggestSpaceUsed();

/*COMPACTACION Y DUMP*/

void compactar();

void dump();




#endif /* SRC_INSTANCIA_H_ */
