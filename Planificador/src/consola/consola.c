#include "consola.h"
#include "../esi/esi.h"

//SIN PARAMETROS
/*
 * El Planificador no le dará nuevas órdenes de
 * ejecución a ningún ESI mientras se encuentre pausado
 * */
void comando_pausar();
/*
 * Reanuda la tarea del planificador
 * */
void comando_continuar();
/*
 * Analiza los deadlocks que existan en el
 * sistema y a que ESI están asociados
 * */
void comando_deadlock();

//CON PARAMETROS
/*
 * Se bloqueará el proceso ESI hasta ser desbloqueado,
 * especificado por dicho <ID> en la cola del recurso <clave>.
 * Cada línea del script a ejecutar es atómica, y no podrá ser interrumpida;
 * si no que se bloqueará en la próxima oportunidad posible. Solo se podrán bloquear
 * de esta manera ESIs que estén en el estado de listo o ejecutando.
 * */
void comando_bloquear_esi_por_id_y_recurso_de_clave(char* id_esi, char* clave);
/*
 * Se desbloqueara el primer proceso ESI bloquedo por la <clave> especificada.
 * Solo se bloqueará ESIs que fueron bloqueados con la consola.
 * */
void comando_desbloquear_primer_esi_por_clave(char* clave);
/*
 * Lista los procesos encolados esperando al <recurso>.
 * */
void comando_listar_processo_por_recurso(char* recurso);
/*
 * Finaliza el proceso con <ID> brindado.
 * Al momento de eliminar el ESI, se debloquearan las claves que tenga tomadas.
 * */
void comando_kill_proceso_esi_por_id(char* id_esi);
/*
 * Con el objetivo de conocer el estado de una <clave>
 * y de probar la correcta distribución de las mismas,
 * el Coordinador permitirá consultar esta información:
 *
 * 1-Valor, en caso de no poseer valor un mensaje que lo indique.
 *
 * 2-Instancia actual en la cual se encuentra la clave.
 * (En caso de que la clave no exista, la Instancia actual debería)
 *
 * 3-Instancia en la cual se guardaría actualmente la clave.
 * (Calcula el valor mediante el algoritmo de distribución,
 *  sin afectar la distribución actual de las claves).
 *
 * 4-ESIs bloqueados a la espera de dicha clave.
 * */
void comando_status_instancias_por_clave(char* clave);

//EXTRAS
/*
 * Termina de ejecutar la consola finalizando el programa
 * */
void comando_exit();
/*
 * Muestra un listado de todos los esi
 * */
void comando_show_esis();
/*
 * Muestra un listado de todos los recursos
 * */
void comando_listar_recursos();

int retorno = CONTINUAR_EJECUTANDO_CONSOLA;


int consolaLeerComando()
{
	size_t size = 20;
	char *entrada = malloc(20);

	getline(&entrada, &size, stdin);
	char** split_comandos = string_split(entrada, " ");
	size_t split_comandos_size = sizeof(split_comandos);
	switch(split_comandos_size)
	{
	case 2:
		switch (getValorByClave(split_comandos[0]))
		{
		case CONSOLA_COMANDO_PAUSAR:
			comando_pausar();
			break;
		case CONSOLA_COMANDO_CONTINUAR:
			comando_continuar();
			break;
		case CONSOLA_COMANDO_DEADLOCK:
			comando_deadlock();
			break;
		case CONSOLA_COMANDO_EXIT:
			comando_exit();
			break;
		case CONSOLA_COMANDO_SHOW:
			comando_show_esis();
			break;
		case CONSOLA_COMANDO_VER_RECURSOS:
			comando_listar_recursos();
			break;
		case CONSOLA_COMANDO_DESCONOCIDO:
			printf("Comando no encontrado");
			break;
		}
		break;
	case 3:
		switch (getValorByClave(split_comandos[0]))
		{
		case CONSOLA_COMANDO_DESBLOQUEAR:
			comando_desbloquear_primer_esi_por_clave(split_comandos[1]);
			break;
		case CONSOLA_COMANDO_LISTAR:
			comando_listar_processo_por_recurso(split_comandos[1]);
			break;
		case CONSOLA_COMANDO_STATUS:
			comando_status_instancias_por_clave(split_comandos[1]);
			break;
		case CONSOLA_COMANDO_KILL:
			comando_kill_proceso_esi_por_id(split_comandos[1]);
			break;
		case CONSOLA_COMANDO_DESCONOCIDO:
			printf("Comando no encontrado");
			break;
		}
		break;
	case 4:
		switch (getValorByClave(split_comandos[0]))
		{
		case CONSOLA_COMANDO_BLOQUEAR:
			comando_bloquear_esi_por_id_y_recurso_de_clave(split_comandos[1], split_comandos[2]);
			break;
		case CONSOLA_COMANDO_DESCONOCIDO:
			printf("Comando no encontrado");
			break;
		}
		break;
	default:
		printf("Comando no encontrado");
		break;
	}

	free(entrada);
	return retorno;
}

void comando_pausar()
{

}

void comando_continuar()
{

}

void comando_deadlock()
{

}

void comando_exit()
{
	printf("¡ADIOS!\n");
	log_info(console_log, "Fin de consola");
	retorno = TERMINAR_CONSOLA;
}

void comando_show_esis()
{
	log_info(console_log, "Consola: Listar ESIs");
	printf("*******************************************\n");
	printf("ESI\t| ESTADO\n");
	printf("-------------------------------------------\n");

	printf("-------------------------------------------\n");
	printf("\n");
}

void comando_bloquear_esi_por_id_y_recurso_de_clave(char* id_esi, char* clave)
{

}

void comando_desbloquear_primer_esi_por_clave(char* clave)
{

}

void comando_listar_processo_por_recurso(char* recurso)
{

}

void comando_kill_proceso_esi_por_id(char* id_esi)
{

}

void comando_status_instancias_por_clave(char* clave)
{

}

void comando_listar_recursos(char * parametro)
{

	int i = 0, tamanioLista = list_size(listaRecursos);
	for (i = 0; i < tamanioLista; i++)
	{
		RECURSO* r = list_get(listaRecursos, i);
		printf("%s\n", r->nombre_recurso);
	}
}

static t_command_struct tabla_referencia_comandos[] = {
		{ "show\n", CONSOLA_COMANDO_SHOW },
		{ "exit\n", CONSOLA_COMANDO_EXIT },
		{ "pause\n", CONSOLA_COMANDO_PAUSAR },
		{ "continue\n", CONSOLA_COMANDO_CONTINUAR },
		{ "bloquear\n", CONSOLA_COMANDO_BLOQUEAR },
		{ "desbloquear\n", CONSOLA_COMANDO_DESBLOQUEAR },
		{ "listar\n", CONSOLA_COMANDO_LISTAR },
		{ "kill\n", CONSOLA_COMANDO_KILL },
		{ "status\n", CONSOLA_COMANDO_STATUS },
		{ "deadlock\n", CONSOLA_COMANDO_DEADLOCK },
		{ "resources\n", CONSOLA_COMANDO_VER_RECURSOS } };

int getValorByClave(char *clave)
{
	int i;
	string_to_lower(clave);
	for (i = 0; i < NCLAVE; i++)
	{
		t_command_struct comando = tabla_referencia_comandos[i];
		if (strcmp(comando.clave, clave) == 0)
			return comando.valor;
	}
	return CONSOLA_COMANDO_DESCONOCIDO;
}
