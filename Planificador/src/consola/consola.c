#include "consola.h"
#include "../esi/esi.h"

bool estaPausado = false;
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
void comando_listar_procesos_por_recurso(char* recurso);
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
 * (En caso de que la clave no se encuentre en una instancia, no se debe mostrar este valor)
 *
 * 3-Instancia en la cual se guardaría actualmente la clave.
 * (Calcula el valor mediante el algoritmo de distribución,
 *  sin afectar la distribución actual de las claves).
 *
 * 4-ESIs bloqueados a la espera de dicha clave.
 * */
void comando_status_instancias_por_clave(char* clave);

int retorno = CONTINUAR_EJECUTANDO_CONSOLA;
void _obtener_todos_los_esis();
void _obtener_todos_los_esis_corriendo();
void _obtener_esis_listos();
void _obtener_esis_ejecutando();
void _obtener_esis_bloqueados();
void _obtener_esis_nuevos();
void _obtener_esis_terminados();
ESI_STRUCT* obtener_esi_por_id(char* id_esi);
status_response_from_coordinator* enviar_status_a_coordinador_por_clave(char* clave);
t_list *hayDeadlock(ESI_STRUCT* esi_original);


void _finalizar_cadena(char *entrada);
char *_obtener_comando(char** split_comandos);
char *_obtener_primer_parametro(char** split_comandos);
char *_obtener_segundo_parametro(char** split_comandos);
void _liberar_comando_y_parametros(char** split_comandos);
bool _estaVacia(char* cadena);
bool _validar_parametro(char* cadena);


int consolaLeerComando()
{
	size_t size = 50;
	char *entrada = malloc(50);

	char *comando = malloc(sizeof(char*));
	char *parametro1 = malloc(sizeof(char*));
	char *parametro2 = malloc(sizeof(char*));
	char** split_comandos = malloc(sizeof(char**));

	fgets(entrada, size, stdin);

	_finalizar_cadena(entrada);
	split_comandos = string_split(entrada, " ");
	comando = _obtener_comando(split_comandos);

	switch (getValorByClave(comando))
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

	case CONSOLA_COMANDO_DESBLOQUEAR:
		parametro1 = _obtener_primer_parametro(split_comandos);
		comando_desbloquear_primer_esi_por_clave(parametro1);
		break;
	case CONSOLA_COMANDO_LISTAR:
		parametro1 = _obtener_primer_parametro(split_comandos);
		comando_listar_procesos_por_recurso(parametro1);
		break;
	case CONSOLA_COMANDO_STATUS:

		parametro1 = _obtener_primer_parametro(split_comandos);
		comando_status_instancias_por_clave(parametro1);
		break;
	case CONSOLA_COMANDO_KILL:
		parametro1 = _obtener_primer_parametro(split_comandos);
		comando_kill_proceso_esi_por_id(parametro1);
		break;

	case CONSOLA_COMANDO_BLOQUEAR:
		parametro1 = _obtener_primer_parametro(split_comandos);
		parametro2 = _obtener_segundo_parametro(split_comandos);
		comando_bloquear_esi_por_id_y_recurso_de_clave(parametro1, parametro2);
		break;
	case CONSOLA_COMANDO_DESCONOCIDO:
		printf("Comando no encontrado\n");
		break;
	}

	//_liberar_comando_y_parametros(split_comandos);
	free(comando);
	free(parametro1);
	free(parametro2);
	//free(split_comandos);
	free(entrada);
	return retorno;
}

void _finalizar_cadena(char *entrada)
{
	if ((strlen(entrada) > 0) && (entrada[strlen (entrada) - 1] == '\n'))
			entrada[strlen (entrada) - 1] = '\0';
}

bool _estaVacia(char* cadena)
{
	return cadena == NULL || string_is_empty(cadena) || string_contains(cadena, " ");
}

bool _validar_parametro(char* cadena)
{
	if(_estaVacia(cadena))
	{
		printf("El parametro esta vacio o no es correcto\n");
		log_info(console_log, "El parametro ingresado es invalido: %s\n", cadena);
		return false;
	}
	return true;
}

char* _obtener_comando(char** split_comandos)
{
	return split_comandos[0];
}

char* _obtener_primer_parametro(char** split_comandos)
{
	return split_comandos[1];
}

char* _obtener_segundo_parametro(char** split_comandos)
{
	return split_comandos[2];
}

void _liberar_comando_y_parametros(char** split_comandos)
{
	if(split_comandos[0] != NULL)
	{
		free(split_comandos[0]);
	}
	if(split_comandos[1] != NULL)
	{
		free(split_comandos[1]);
	}
	if(split_comandos[2] != NULL)
	{
		free(split_comandos[2]);
	}
	if(split_comandos != NULL)
	{
		free(split_comandos);
	}
}

void comando_pausar()
{
	log_info(console_log, "Consola: Pausar\n");
	if(!estaPausado)
	{
		pthread_mutex_lock(&mutexPlanificador);
		estaPausado = true;
		printf("Planificador pausado!\n");
	} else {
		printf("El planificador ya se encuentra pausado!\n");
	}
}

void comando_continuar()
{
	log_info(console_log, "Consola: Continuar\n");
	if(estaPausado)
	{
		pthread_mutex_unlock(&mutexPlanificador);
		estaPausado = false;
		printf("Planificador reanudado!\n");
	} else {
		printf("El planificador no se encuentra pausado!\n");
	}
}

void comando_bloquear_esi_por_id_y_recurso_de_clave(char* id_esi, char* clave)
{
	log_info(console_log, "Consola: Bloquear id_esi: %s - clave: %s\n", id_esi, clave);
	_validar_parametro(clave);
	_validar_parametro(id_esi);

	if(_validar_parametro(clave) && _validar_parametro(id_esi))
	{
		pthread_mutex_lock(&mutexPlanificador);
		_obtener_esis_nuevos();
		_obtener_esis_listos();
		_obtener_esis_ejecutando();

		ESI_STRUCT* esi = obtener_esi_por_id(id_esi);

		if(esi != NULL)
		{
			if(esi->id == esiEjecutando->id)
			{
				printf("No se puede bloquear el ESI en ejecucion\n");
			} else
			{
				bloquearEsi(esi->id, clave);
				printf("Se ha bloqueado el ESI con id_esi: %s para la clave: %s\n", id_esi, clave);
			}
		} else {
			printf("No se encontro nignun proceso esi para bloquear con id_esi: %s para la clave: %s\n", id_esi, clave);
			log_info(console_log, "No existe proceso esi para bloquear con id_esi: %s para la clave: %s\n", id_esi, clave);
		}

		list_clean(listaEsis);
		pthread_mutex_unlock(&mutexPlanificador);
	}
}

void comando_desbloquear_primer_esi_por_clave(char* clave)
{
	log_info(console_log, "Consola: Desbloquear clave: %s\n", clave);

	if(_validar_parametro(clave))
	{
		pthread_mutex_lock(&mutexPlanificador);

		if(estadoRecurso(clave) == RECURSO_BLOQUEADO)
		{
			liberarRecurso(clave);
			printf("Se ha desbloqueado el recurso con la clave: %s\n", clave);
		} else {
			printf("El recurso no se encuentra bloqueado\n");
		}

		pthread_mutex_unlock(&mutexPlanificador);
	}
}

void comando_listar_procesos_por_recurso(char* recurso)
{
	log_info(console_log, "Consola: Listar %s\n", recurso);
	void _list_esis(ESI_STRUCT *e)
	{
		char * estado = malloc(sizeof(char*));
		switch (e->estado)
		{
		case ESI_LISTO:
			strcpy(estado, "LISTO\0");
			break;
		case ESI_EJECUTANDO:
			strcpy(estado, "EJECUTANDO\0");
			break;
		case ESI_BLOQUEADO:
			strcpy(estado, "BLOQUEADO\0");
			break;
		case ESI_TERMINADO:
			strcpy(estado, "TERMINADO\0");
			break;
		}
		printf("%d\t| %s\n", e->id, estado);
		free(estado);
	}

	if(_validar_parametro(recurso))
	{
		_obtener_todos_los_esis();

		bool _espera_por_recurso(ESI_STRUCT* esi)
		{
			if(esi->informacionDeBloqueo == NULL)
				return false;
			return string_equals_ignore_case(esi->informacionDeBloqueo->recursoNecesitado, recurso);
		}

		t_list* esis_filtrados = list_filter(listaEsis, (void*) _espera_por_recurso);
		if(!list_is_empty(esis_filtrados)) {
			printf("ID_ESI\t| ESTADO\n");
			list_iterate(esis_filtrados, (void*) _list_esis);
		} else {
			printf("No se encuentran procesos esi esperando por el recurso: %s\n", recurso);
			log_info(console_log, "Sin procesos esi esperando por el recurso: %s\n", recurso);
		}
		list_destroy(esis_filtrados);
		list_clean(listaEsis);
	}
}

void comando_deadlock() 
{
	log_info(console_log, "Consola: Deadlock\n");

	int i;
	for (i = 0; i < list_size(listaEsiBloqueados); i++)
	{
		ESI_STRUCT *esi = list_get(listaEsiBloqueados, i);
		listaEsiEnDeadlock = hayDeadlock(esi);

		if(listaEsiEnDeadlock != NULL)
		{
			// 1) RECORRER LA LISTA E IMPRIMIR POR PANTALLA
			if(!list_is_empty(listaEsiEnDeadlock))
			{
				printf("Los siguientes esis estan en deadlock: \n");
				printf("Esi %d\n",esi->id);
				int j;
				for (j = 0; j < list_size(listaEsiEnDeadlock) ; j++)
				{
					int* aux = list_get(listaEsiEnDeadlock, j);
					printf("Bloqueado por %d\n", *aux);
				}
				list_clean(listaEsiEnDeadlock);
				// 2) Encontro deadlock => retornar
				//return;
			} else {
				printf("No se ha detectado deadlock\n");
			}
		}
	}
}

t_list *hayDeadlock(ESI_STRUCT* esi_original)
{
	t_list* lista = list_create();

	RECURSO * recursoQueNecesitaEsiBloqueante = NULL;
	RECURSO * recursoQueNecesitaEsiOriginal = getRecurso(
			esi_original->informacionDeBloqueo->recursoNecesitado);

	if (recursoQueNecesitaEsiOriginal->estado == RECURSO_LIBRE)
		return NULL;

	// Este es el esi que esta bloqueando a mi esi original
	ESI_STRUCT *esiBloqueante = recursoQueNecesitaEsiOriginal->esi_bloqueante;

	// Si no hay esi bloqueante => retorno
	if (esiBloqueante == NULL)
		return NULL;

	list_add(lista, &esiBloqueante->id);

	while (true) {
		if (esiBloqueante->estado == ESI_BLOQUEADO) {
			// Este es el recurso que necesita el esi bloqueante para continuar
			recursoQueNecesitaEsiBloqueante = getRecurso(
					esiBloqueante->informacionDeBloqueo->recursoNecesitado);

			// Si el recurso que necesita el esi bloqueante esta bloqueado por el esi original => hay deadlock
			if (recursoQueNecesitaEsiBloqueante->esi_bloqueante->id
					== esi_original->id) {
				list_add(lista, &esi_original->id);

				return lista;
			}

			if(esiBloqueante->id == recursoQueNecesitaEsiBloqueante->esi_bloqueante->id)
				return lista;

			// No estaba bloqueado por el original => guardo la info
			list_add(lista, &recursoQueNecesitaEsiBloqueante->esi_bloqueante->id);

			// Evaluo al esi que bloquea a mi esi bloqueante
			esiBloqueante = recursoQueNecesitaEsiBloqueante->esi_bloqueante;
		} else {
			// El esi bloqueante no esta bloqueado
			return NULL;
		}
	}
}

void comando_kill_proceso_esi_por_id(char* id_esi) {
	log_info(console_log, "Consola: Kill %s\n", id_esi);

	if(_validar_parametro(id_esi))
	{
		int i = 0;

		if(esiEjecutando != NULL && string_equals_ignore_case(id_esi, string_itoa(esiEjecutando->id)))
		{
			printf("No se puede bloquear el ESI en ejecucion\n");
			return;
		}

		for(i = 0; i < list_size(listaEsiListos); i++)
		{
			ESI_STRUCT* esi = list_get(listaEsiListos, i);
			if(string_equals_ignore_case(string_itoa(esi->id), id_esi))
			{
				matarEsi(esi);

				esi->estado = ESI_TERMINADO;
				list_remove(listaEsiListos, i);
				list_add(listaEsiTerminados, clonarEsi(esi));

				printf("Se ha eliminado el ESI con id_esi: %d\n", esi->id);

				return;
			}
		}
		for(i = 0; i < list_size(listaEsiBloqueados); i++)
		{
			ESI_STRUCT* esi = list_get(listaEsiBloqueados, i);
			if(string_equals_ignore_case(string_itoa(esi->id), id_esi))
			{
				matarEsi(esi);

				esi->estado = ESI_TERMINADO;
				list_remove(listaEsiBloqueados, i);
				list_add(listaEsiTerminados, clonarEsi(esi));

				printf("Se ha eliminado el ESI con id_esi: %d\n", esi->id);

				return;
			}
		}
		for(i = 0; i < list_size(listaEsiNuevos); i++)
		{
			ESI_STRUCT* esi = list_get(listaEsiNuevos, i);
			if(string_equals_ignore_case(string_itoa(esi->id), id_esi))
			{
				matarEsi(esi);

				esi->estado = ESI_TERMINADO;
				list_remove(listaEsiNuevos, i);
				list_add(listaEsiTerminados, clonarEsi(esi));

				printf("Se ha eliminado el ESI con id_esi: %d\n", esi->id);

				return;
			}
		}

		printf("No se encontro proceso esi con id: %s especificado \n", id_esi);
		log_info(console_log, "No existe proceso esi con el id: %s\n", id_esi);
	}
}

ESI_STRUCT* obtener_esi_por_id(char* id_esi)
{
	int _es_esi_unico(ESI_STRUCT *e)
	{
		return string_equals_ignore_case(string_itoa(e->id), id_esi);
	}

	ESI_STRUCT* esi = list_find(listaEsis, (void*) _es_esi_unico);
	return esi;
}

void comando_status_instancias_por_clave(char* clave)
{
	log_info(console_log, "Consola: Status %s\n", clave);
	if(_validar_parametro(clave))
	{
		char * valor_clave;
		status_response_from_coordinator* response = enviar_status_a_coordinador_por_clave(clave);
		if (response != NULL){
			printf("VALOR\t| INSTANCIA_ACTUAL\t| INSTANCIA_A_GUARDAR\t| ESIS_BLOQUEADOS_POR_CLAVE\n");
			// Debo esperar al valor de la clave , ya que existe
			if (response->payload_valor_size > 0){
				valor_clave = malloc(response->payload_valor_size);
				if (recv(coordinator_socket_console, valor_clave, response->payload_valor_size, MSG_WAITALL) != response->payload_valor_size) {
					printf("Ocurrio un error de comunicacion con el Coordinador al recuperar el valor de la clave!\n");
					exit(1);
				}

			} else {
				valor_clave = malloc(strlen("La clave no posee valor"));
				strcpy(valor_clave, "La clave no posee valor");
			}
			_obtener_esis_bloqueados();

			bool _espera_por_recurso(ESI_STRUCT* esi)
			{
				return string_equals_ignore_case(esi->informacionDeBloqueo->recursoNecesitado, clave);
			}
			t_list* esis_filtrados = list_filter(listaEsis, (void*) _espera_por_recurso);

			char * lista_esis_bloqueados = malloc(strlen("No hay ESIS bloquados a la espera de la clave"));
			if(!list_is_empty(esis_filtrados))
			{
				int j;
				for (j = 0; j < list_size(esis_filtrados); j++)
				{
					ESI_STRUCT* aux = (ESI_STRUCT*) list_get(esis_filtrados, j);
					if (j == list_size(esis_filtrados) - 1) {
						strcat(lista_esis_bloqueados, aux->id + "\n ");
					} else {
						strcat(lista_esis_bloqueados, aux->id + ", ");
					}
				}
			} else {
				strcpy(lista_esis_bloqueados, "No hay ESIS bloquados a la espera de la clave");
			}
			printf("%s\t| %s\t| %s\t| %s\n",
									valor_clave,
									response->nombre_intancia_actual,
									response->nombre_intancia_posible,
									lista_esis_bloqueados);
			log_info(console_log, "%s\t| %s\t| %s\t| %s\n", valor_clave,
															response->nombre_intancia_actual,
															response->nombre_intancia_posible,
															lista_esis_bloqueados);

			free(valor_clave);
			free(lista_esis_bloqueados);
			list_destroy(esis_filtrados);
			list_clean(listaEsis);
		}else{
			printf("No se obtuvo respuesta del Coordinador para la solicitud de status de la clave: %s\n" , clave);
			log_info(console_log, "No se obtuvo respuesta del Coordinador para la solicitud\n");
		}
	}
}

status_response_from_coordinator* enviar_status_a_coordinador_por_clave(char* clave) {
	/*Debo enviar la clave al Coordinador*/
	status_response_from_coordinator* coordinator_response = NULL;
	int payload_size = strlen(clave) + 1;
	/*Envio tamanio de la clave*/
	if (send(coordinator_socket_console, &payload_size, sizeof(payload_size), 0) != 4) {
		printf("FALLO AL ENVIAR TAMANIO KEY\n");
	}

	/*Envio clave*/
	if (send(coordinator_socket_console, clave, payload_size, 0) != payload_size) {
		printf("FALLO AL ENVIAR\n");
	}
	/*Espero la respuesta*/
	void *status_buffer = malloc(STATUS_RESPONSE_FROM_COORDINATOR);
	int res = recv(coordinator_socket_console, status_buffer,
				STATUS_RESPONSE_FROM_COORDINATOR, MSG_WAITALL);
	if (res < STATUS_RESPONSE_FROM_COORDINATOR) {
		printf("OCURRIO UN ERROR AL RECIBIR LA RESPUESTA\n");
		return coordinator_response;
	}
	coordinator_response = derialize_status_response_from_coordinator(status_buffer);

	return coordinator_response;
}

void _obtener_todos_los_esis()
{
	_obtener_esis_listos();
	_obtener_esis_ejecutando();
	_obtener_esis_nuevos();
	_obtener_esis_bloqueados();
	_obtener_esis_terminados();
}

void _obtener_todos_los_esis_corriendo()
{
	_obtener_esis_listos();
	_obtener_esis_ejecutando();
	_obtener_esis_nuevos();
	_obtener_esis_bloqueados();
}

void _obtener_esis_listos()
{
	list_add_all(listaEsis, listaEsiListos);
}

void _obtener_esis_ejecutando()
{
	if(esiEjecutando != NULL)
		list_add(listaEsis, esiEjecutando);
}

void _obtener_esis_bloqueados()
{
	list_add_all(listaEsis, listaEsiBloqueados);
}

void _obtener_esis_nuevos()
{
	list_add_all(listaEsis, listaEsiNuevos);
}

void _obtener_esis_terminados()
{
	list_add_all(listaEsis, listaEsiTerminados);
}

static t_command_struct tabla_referencia_comandos[] = {
		{ "pausar", CONSOLA_COMANDO_PAUSAR },
		{ "continuar", CONSOLA_COMANDO_CONTINUAR },
		{ "bloquear", CONSOLA_COMANDO_BLOQUEAR },
		{ "desbloquear", CONSOLA_COMANDO_DESBLOQUEAR },
		{ "listar", CONSOLA_COMANDO_LISTAR },
		{ "kill", CONSOLA_COMANDO_KILL },
		{ "status", CONSOLA_COMANDO_STATUS },
		{ "deadlock", CONSOLA_COMANDO_DEADLOCK } };

int getValorByClave(char *clave)
{
	int i;

	if(clave == NULL)
		return CONSOLA_COMANDO_DESCONOCIDO;

	string_to_lower(clave);
	for (i = 0; i < NCLAVE; i++)
	{
		t_command_struct comando = tabla_referencia_comandos[i];
		if (strcmp(comando.clave, clave) == 0)
			return comando.valor;
	}
	return CONSOLA_COMANDO_DESCONOCIDO;
}
