#ifndef SRC_CONSOLA_CONSOLA_H_
#define SRC_CONSOLA_CONSOLA_H_

#include "../commons/commons.h"

#define CONSOLA_COMANDO_DESCONOCIDO -1
#define CONSOLA_COMANDO_MOSTRAR 1
#define CONSOLA_COMANDO_SALIR 2
#define CONSOLA_COMANDO_VER_RECURSOS 3
#define CONSOLA_COMANDO_PAUSAR 4
#define CONSOLA_COMANDO_CONTINUAR 5
#define CONSOLA_COMANDO_BLOQUEAR 6
#define CONSOLA_COMANDO_DESBLOQUEAR 7
#define CONSOLA_COMANDO_LISTAR 8
#define CONSOLA_COMANDO_KILL 9
#define CONSOLA_COMANDO_STATUS 10
#define CONSOLA_COMANDO_DEADLOCK 11

typedef struct {
	char *clave;
	int valor;
} t_command_struct;

#define NCLAVE (sizeof(tabla_referencia_comandos)/sizeof(t_command_struct))

#define CONTINUAR_EJECUTANDO_CONSOLA 0
#define TERMINAR_CONSOLA 1

int consolaLeerComando();
int getValorByClave(char *clave);

#endif /* SRC_CONSOLA_CONSOLA_H_ */
