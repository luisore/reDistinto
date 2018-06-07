#include <commons/config.h>
#include <commons/log.h>
#include <stdio.h> //printf
#include <commons/collections/list.h>
#include <commons/string.h>
#include "libs/tcpserver.h"
#include "libs/protocols.h"
#include <pthread.h>

#ifndef CONSOLA_INSTANCIA_H_
#define CONSOLA_INSTANCIA_H_

int instance_socket;

typedef struct {
	char instance_name[30];
	instance_type_e instance_type;
	int socket_id;
} t_connected_client;


void print_header();
void print_goodbye();
void exit_program(int);


void _finalizar_cadena(char *entrada);
char* _obtener_comando(char** split_comandos);
char* _obtener_primer_parametro(char** split_comandos);
char* _obtener_segundo_parametro(char** split_comandos);
int getValorByClave(char *clave);
void operacion_get(char * clave);
void operacion_set(char * parametro1 , char * parametro2);
void operacion_store(char * clave);

int consola_leer();
void input();

#endif /* CONSOLA_INSTANCIA_H_ */
