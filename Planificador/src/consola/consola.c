#include "consola.h"
#include "../esi/esi.h"

void comandoVerRecursosBloqueados();
void comandoBloquearEsi(char * instruccion);
void comandoDesbloquearEsi(char * instruccion);
void comandoVerProcesosParaRecurso(char * instruccion);
void comandoMatarEsi(char * instruccion);
void comandoVerEstadoClave(char * instruccion);
void comandoDeadlock();

int consolaLeerComando(t_log * console_log) {
	int retorno = CONTINUAR_EJECUTANDO_CONSOLA;
	size_t size = 20;
	char *entrada = malloc(20);

	getline(&entrada, &size, stdin);

	/* show no lo pide el enunciado. lo agrego a modo de prueba*/
	if (string_equals_ignore_case(entrada, CONSOLA_COMANDO_MOSTRAR)) {
		listarEsi(console_log);

	} else if (string_equals_ignore_case(entrada,
			CONSOLA_COMANDO_VER_RECURSOS)) {
		comandoVerRecursosBloqueados();

	} else if (string_equals_ignore_case(entrada, CONSOLA_COMANDO_PAUSAR)) {
		// TODO: PAUSAR

	} else if (string_equals_ignore_case(entrada, CONSOLA_COMANDO_CONTINUAR)) {
		// TODO: CONTINUAR

	} else if (string_contains(entrada, CONSOLA_COMANDO_BLOQUEAR)) {
		comandoBloquearEsi(entrada);

	} else if (string_contains(entrada, CONSOLA_COMANDO_DESBLOQUEAR)) {
		comandoDesbloquearEsi(entrada);

	} else if (string_contains(entrada, CONSOLA_COMANDO_LISTAR)) {
		comandoVerProcesosParaRecurso(entrada);

	} else if (string_contains(entrada, CONSOLA_COMANDO_STATUS)) {
		comandoVerEstadoClave(entrada);

	} else if (string_contains(entrada, CONSOLA_COMANDO_KILL)) {
		comandoMatarEsi(entrada);

	} else if (string_contains(entrada, CONSOLA_COMANDO_DEADLOCK)) {
		comandoDeadlock();

	} else if (string_equals_ignore_case(entrada, CONSOLA_COMANDO_SALIR)) {
		printf("¡ADIOS!\n");
		log_error(console_log, "Fin de consola");
		retorno = TERMINAR_CONSOLA;

	} else {
		printf("Comando no encontrado");
	}

	free(entrada);
	return retorno;
}

void comandoVerRecursosBloqueados() {
	int i = 0, tamanioLista = list_size(listaRecursos);

	for (i = 0; i < tamanioLista; i++) {
		RECURSO* r = list_get(listaRecursos, i);

		printf("%s\n", r->nombre_recurso);
	}
}

void comandoBloquearEsi(char * instruccion) {
}

void comandoDesbloquearEsi(char * instruccion) {
}

void comandoVerProcesosParaRecurso(char * instruccion) {
}

void comandoMatarEsi(char * instruccion) {
}

void comandoVerEstadoClave(char * instruccion) {
}

void comandoDeadlock() {
}
