#ifndef SRC_CONSOLA_CONSOLA_H_
#define SRC_CONSOLA_CONSOLA_H_

#include "../commons/commons.h"

#define CONSOLA_COMANDO_DESCONOCIDO -1
#define CONSOLA_COMANDO_SHOW 1
#define CONSOLA_COMANDO_EXIT 2
#define CONSOLA_COMANDO_PAUSAR 3
#define CONSOLA_COMANDO_CONTINUAR 4
#define CONSOLA_COMANDO_BLOQUEAR 5
#define CONSOLA_COMANDO_DESBLOQUEAR 6
#define CONSOLA_COMANDO_LISTAR 7
#define CONSOLA_COMANDO_KILL 8
#define CONSOLA_COMANDO_STATUS 9
#define CONSOLA_COMANDO_DEADLOCK 10
#define CONSOLA_COMANDO_VER_RECURSOS 11

typedef struct {
	char *clave;
	int valor;
} t_command_struct;

#define NCLAVE (sizeof(tabla_referencia_comandos)/sizeof(t_command_struct))

#define CONTINUAR_EJECUTANDO_CONSOLA 0
#define TERMINAR_CONSOLA 1

int consolaLeerComando();
int getValorByClave(char *clave);


void _list_recursos(RECURSO *r);
void _list_esis(ESI_STRUCT *e);
bool _espera_por_recurso(ESI_STRUCT* esi, char* recurso);

#endif /* SRC_CONSOLA_CONSOLA_H_ */
