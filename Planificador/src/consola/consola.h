#ifndef SRC_CONSOLA_CONSOLA_H_
#define SRC_CONSOLA_CONSOLA_H_

#define CONSOLA_COMANDO_MOSTRAR "show\n"
#define CONSOLA_COMANDO_SALIR "exit\n"

#define CONTINUAR_EJECUTANDO_CONSOLA 0
#define TERMINAR_CONSOLA 1

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>

int consolaLeerComando(t_log * console_log);

#endif /* SRC_CONSOLA_CONSOLA_H_ */
