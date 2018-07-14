#ifndef SRC_CONSOLA_CONSOLA_H_
#define SRC_CONSOLA_CONSOLA_H_

#include "../commons/commons.h"
#include "../libs/protocols.h"

#define CONSOLA_COMANDO_DESCONOCIDO -1
#define CONSOLA_COMANDO_PAUSAR 1
#define CONSOLA_COMANDO_CONTINUAR 2
#define CONSOLA_COMANDO_BLOQUEAR 3
#define CONSOLA_COMANDO_DESBLOQUEAR 4
#define CONSOLA_COMANDO_LISTAR 5
#define CONSOLA_COMANDO_KILL 6
#define CONSOLA_COMANDO_STATUS 7
#define CONSOLA_COMANDO_DEADLOCK 8

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
