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

	switch (string_to_lower(entrada)) {
	case CONSOLA_COMANDO_MOSTRAR:
		/* show no lo pide el enunciado. lo agrego a modo de prueba*/
		listarEsi(console_log);
		break;
	case CONSOLA_COMANDO_VER_RECURSOS:
		comandoVerRecursosBloqueados();
		break;
	case CONSOLA_COMANDO_PAUSAR:
		// TODO: PAUSAR
		break;
	case CONSOLA_COMANDO_CONTINUAR:
		// TODO: CONTINUAR
		break;
	case CONSOLA_COMANDO_BLOQUEAR:
		comandoBloquearEsi(entrada);
		break;
	case CONSOLA_COMANDO_DESBLOQUEAR:
		comandoDesbloquearEsi(entrada);
		break;
	case CONSOLA_COMANDO_LISTAR:
		comandoVerProcesosParaRecurso(entrada);
		break;
	case CONSOLA_COMANDO_STATUS:
		comandoVerEstadoClave(entrada);
		break;
	case CONSOLA_COMANDO_KILL:
		comandoMatarEsi(entrada);
		break;
	case CONSOLA_COMANDO_DEADLOCK:
		comandoDeadlock();
		break;
	case CONSOLA_COMANDO_SALIR:
		printf("¡ADIOS!\n");
		log_error(console_log, "Fin de consola");
		retorno = TERMINAR_CONSOLA;
		break;
	default:
		printf("Comando no encontrado");
		break;
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
