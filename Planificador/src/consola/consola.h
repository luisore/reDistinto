#ifndef SRC_CONSOLA_CONSOLA_H_
#define SRC_CONSOLA_CONSOLA_H_

#define CONSOLA_COMANDO_MOSTRAR "show\n"
#define CONSOLA_COMANDO_SALIR "exit\n"
#define CONSOLA_COMANDO_VER_RECURSOS "listar --all\n"
#define CONSOLA_COMANDO_PAUSAR "pause\n"
#define CONSOLA_COMANDO_CONTINUAR "continue\n"
#define CONSOLA_COMANDO_BLOQUEAR "bloquear"
#define CONSOLA_COMANDO_DESBLOQUEAR "desbloquear"
#define CONSOLA_COMANDO_LISTAR "listar"
#define CONSOLA_COMANDO_KILL "kill"
#define CONSOLA_COMANDO_STATUS "status"
#define CONSOLA_COMANDO_DEADLOCK "deadlock"


#define CONTINUAR_EJECUTANDO_CONSOLA 0
#define TERMINAR_CONSOLA 1

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>

int consolaLeerComando(t_log * console_log);

#endif /* SRC_CONSOLA_CONSOLA_H_ */
