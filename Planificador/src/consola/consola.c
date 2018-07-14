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
ESI_STRUCT* obtener_esi_por_clave_recurso(char* clave);
status_response_from_coordinator* enviar_status_a_coordinador_por_clave(char* clave);
void _verificar_si_alguien_tiene_el_recurso(char* clave);
bool _verificar_si_hay_circulo();
bool _list_element_repeats(t_list* self, bool (*comparator)(void *,void *));


void _finalizar_cadena(char *entrada);
char *_obtener_comando(char** split_comandos);
char *_obtener_primer_parametro(char** split_comandos);
char *_obtener_segundo_parametro(char** split_comandos);
void _liberar_comando_y_parametros(char** split_comandos);
bool _estaVacia(char* cadena);
bool _validar_parametro(char* cadena);


int consolaLeerComando()
{
	size_t size = 20;
	char *entrada = malloc(20);

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
	pthread_mutex_lock(&mutexPrincipal);
	printf("Planificador pausado!\n");
}

void comando_continuar()
{
	log_info(console_log, "Consola: Continuar\n");
	pthread_mutex_unlock(&mutexPrincipal);
	printf("Planificador reanudado!\n");
}

void comando_bloquear_esi_por_id_y_recurso_de_clave(char* id_esi, char* clave)
{
	log_info(console_log, "Consola: Bloquear id_esi: %s - clave: %s\n", id_esi, clave);
	_validar_parametro(clave);
	_validar_parametro(id_esi);

	if(_validar_parametro(clave) && _validar_parametro(id_esi))
	{
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
				pthread_mutex_lock(&mutexPrincipal);
				bloquearEsi(esi->id, clave);
				pthread_mutex_unlock(&mutexPrincipal);
				printf("Se ha bloqueado el ESI con id_esi: %s para la clave: %s\n", id_esi, clave);
			}
		} else {
			printf("No se encontro nignun proceso esi para bloquear con id_esi: %s para la clave: %s\n", id_esi, clave);
			log_info(console_log, "No existe proceso esi para bloquear con id_esi: %s para la clave: %s\n", id_esi, clave);
		}

		list_clean(listaEsis);
	}
}

void comando_desbloquear_primer_esi_por_clave(char* clave)
{
	log_info(console_log, "Consola: Desbloquear clave: %s\n", clave);

	if(_validar_parametro(clave))
	{
		_obtener_esis_bloqueados();

		ESI_STRUCT* esi = obtener_esi_por_clave_recurso(clave);
		if(esi != NULL)
		{
			pthread_mutex_lock(&mutexPrincipal);
			desbloquearEsi(esi);
			pthread_mutex_unlock(&mutexPrincipal);
			printf("Se ha desbloqueado el ESI con id_esi: %d para la clave: %s\n", esi->id, clave);
		} else {
			printf("No se encontro nignun proceso esi bloquedo por la clave: %s especificada\n", clave);
			log_info(console_log, "No existe proceso esi bloquedo por la clave: %s\n", clave);
		}

		list_clean(listaEsis);
	}
}

bool _espera_por_recurso(ESI_STRUCT* esi, char* recurso)
{
	return string_equals_ignore_case(esi->informacionDeBloqueo->recursoNecesitado, recurso);
}

ESI_STRUCT* obtener_esi_por_clave_recurso(char* clave)
{
	ESI_STRUCT* esi = list_find(listaEsis, (void*) _espera_por_recurso);
	return esi;
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
	int cantidad_de_bloqueados = list_size(listaEsiBloqueados);
	listaEsiEnDeadlock = list_create();

	if (cantidad_de_bloqueados < 2) {
		printf("No se encontro ningun Deadlock.\n");
		return;
	}

	for (i = 0; i < cantidad_de_bloqueados; i++) {
		ESI_STRUCT* esi_actual = (ESI_STRUCT*) list_get(listaEsiBloqueados, i);
		char* clave_que_necesita = malloc(
				strlen(esi_actual->informacionDeBloqueo->recursoNecesitado)
						+ 1);
		strcpy(clave_que_necesita,
				esi_actual->informacionDeBloqueo->recursoNecesitado);

		if (!_verificar_si_hay_circulo()) {
			list_clean(listaEsiEnDeadlock);
			_verificar_si_alguien_tiene_el_recurso(clave_que_necesita);
		}

		free(clave_que_necesita);

		if (_verificar_si_hay_circulo()) {
			list_remove(listaEsiEnDeadlock, list_size(listaEsiEnDeadlock) - 1);
			if (list_size(listaEsiEnDeadlock) == 1) {
				printf("No se encontro ningun Deadlock.\n");
				return;
			}
		} else {
			printf("No se encontro ningun Deadlock.\n");
			return;
		}

	}
	printf("Los siguientes esis estan en deadlock: ");
	int j;
	for (j = 0; j < list_size(listaEsiEnDeadlock); j++) {
		ESI_STRUCT* aux = (ESI_STRUCT*) list_get(listaEsiEnDeadlock, j);
		if (j == list_size(listaEsiEnDeadlock) - 1) {
			printf("%d.\n ", aux->id);
		} else {
			printf("%d, ", aux->id);
		}
	}
	list_destroy(listaEsiEnDeadlock);

}

void _verificar_si_alguien_tiene_el_recurso(char* clave)
{
	int k;
	int cantidad_de_bloqueados = list_size(listaEsiBloqueados);
	for (k = 0; k < cantidad_de_bloqueados ; k++) {
		ESI_STRUCT* esi_actual = (ESI_STRUCT*) list_get(listaEsiBloqueados,k);

		if(_espera_por_recurso(esi_actual,clave)){
			list_add(listaEsiEnDeadlock, esi_actual);
			if(!_verificar_si_hay_circulo()){
				_verificar_si_alguien_tiene_el_recurso(esi_actual->informacionDeBloqueo->recursoNecesitado);
			}
		}
	}
}

bool _verificar_si_hay_circulo()
{
	return _list_element_repeats(listaEsiEnDeadlock,(void*)sonIguales);
}


bool _list_element_repeats(t_list* self, bool (*comparator)(void *,void *))
{
	int longitud_de_lista = list_size(self);
	int i,j;
	if(longitud_de_lista > 1){
		for(i = 0; i < longitud_de_lista; i++ ){
			for(j = 0; j < longitud_de_lista; j++ ){
				if( (comparator(list_get(self,i),list_get(self,j))) && i != j ){
					return true;
				}
			}
		}
	}
	return false;
}

void comando_kill_proceso_esi_por_id(char* id_esi) {
	log_info(console_log, "Consola: Kill %s\n", id_esi);

	if(_validar_parametro(id_esi))
	{
		_obtener_todos_los_esis_corriendo();

		ESI_STRUCT* esi = obtener_esi_por_id(id_esi);

		if(esi != NULL)
		{
			if(esi->id == esiEjecutando->id)
			{
				printf("No se puede bloquear el ESI en ejecucion\n");
			} else
			{
				matarEsi(esi);
				printf("Se ha elimiando el ESI con id_esi: %d\n", esi->id);
			}
		} else {
			printf("No se encontro proceso esi con id: %s especificado \n", id_esi);
			log_info(console_log, "No existe proceso esi con el id: %s\n", id_esi);
		}
		list_clean(listaEsis);
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

//TODO
void comando_status_instancias_por_clave(char* clave)
{
	log_info(console_log, "Consola: Status %s\n", clave);
	if(_validar_parametro(clave))
	{
		char * valor_clave;
		status_response_from_coordinator* response = enviar_status_a_coordinador_por_clave(clave);
		if (response != NULL){
			// Debo esperar al valor de la clave , ya que existe
			printf("VALOR\t| INSTANCIA_ACTUAL\t| INSTANCIA_A_GUARDAR\t| ESIS_BLOQUEADOS_POR_CLAVE\n");
			if (response->payload_valor_size > 0){
				valor_clave = malloc(response->payload_valor_size);
				if (recv(coordinator_socket_console, valor_clave, response->payload_valor_size, MSG_WAITALL) != response->payload_valor_size) {
					_obtener_esis_bloqueados();
					t_list* esis_filtrados = list_filter(listaEsis, (void*) _espera_por_recurso);
					free(valor_clave);
					list_destroy(esis_filtrados);
					// TODO - Verificar con Pablo que se debe hacer aqui.
				}
				// Aqui ya deberiamos tener el valor
			}

		}else{
			printf("No se obtuvo respuesta del Coordinador para la solicitud de status de la clave: %s\n" , clave);
			log_info(console_log, "No se obtuvo respuesta del Coordinador para la solicitud\n");
		}

		list_clean(listaEsis);
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
