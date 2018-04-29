#include "consola.h"
#include "../esi/esi.h"

int consolaLeerComando(t_log * console_log){
	int retorno = CONTINUAR_EJECUTANDO_CONSOLA;
	size_t size = 20;
	char *entrada = malloc(20);

	getline(&entrada, &size, stdin);

	/* show no lo pide el enunciado. lo agrego a modo de prueba*/
	if (string_equals_ignore_case(entrada, CONSOLA_COMANDO_MOSTRAR)) {
		listarEsi(console_log);
	}
	else if (string_equals_ignore_case(entrada, CONSOLA_COMANDO_SALIR)) {
		printf("¡ADIOS!\n");
		log_error(console_log, "Fin de consola");
		retorno = TERMINAR_CONSOLA;
	}

	free(entrada);
	return retorno;
}
