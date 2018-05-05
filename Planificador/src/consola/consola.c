#include "consola.h"
#include "../esi/esi.h"

void comandoPausar();
void comandoContinuar();
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

	switch (getValorByClave(entrada)) {
	case CONSOLA_COMANDO_MOSTRAR:
		/* show no lo pide el enunciado. lo agrego a modo de prueba*/
		listarEsi(console_log);
		break;
	case CONSOLA_COMANDO_VER_RECURSOS:
		comandoVerRecursosBloqueados();
		break;
	case CONSOLA_COMANDO_PAUSAR:
		comandoPausar();
		break;
	case CONSOLA_COMANDO_CONTINUAR:
		comandoContinuar();
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

void comandoPausar() {
}

void comandoContinuar() {
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

static t_command_struct tabla_referencia_comandos[] = {
		{ "show\n", CONSOLA_COMANDO_MOSTRAR },
		{ "exit\n", CONSOLA_COMANDO_SALIR },
		{ "listar --all\n", CONSOLA_COMANDO_VER_RECURSOS },
		{ "pause\n", CONSOLA_COMANDO_PAUSAR },
		{ "continue\n", CONSOLA_COMANDO_CONTINUAR },
		{ "bloquear\n", CONSOLA_COMANDO_BLOQUEAR },
		{ "desbloquear\n", CONSOLA_COMANDO_DESBLOQUEAR },
		{ "listar\n", CONSOLA_COMANDO_LISTAR },
		{ "kill\n", CONSOLA_COMANDO_KILL },
		{ "status\n", CONSOLA_COMANDO_STATUS },
		{ "deadlock\n", CONSOLA_COMANDO_DEADLOCK } };

int getValorByClave(char *clave) {
	int i;
	for (i = 0; i < NCLAVE; i++) {
		t_command_struct *sym = tabla_referencia_comandos + i * sizeof(t_command_struct);
		string_to_lower(clave);
		if (strcmp(sym->clave, clave) == 0)
			return sym->valor;
	}
	return CONSOLA_COMANDO_DESCONOCIDO;
}
