#include "planificacion.h"
#include "../esi/esi.h"

// PRIVADAS
bool hayDesalojo = false;
float alpha = 0.5f;
int estimacionInicial = 0;
int algoritmo = 0; // 0: SJF_SD; 1: SJF_CD; 2: HRRN

void chequearDesbloqueos();
void admitirNuevosEsis();


// SETTERS
void setAlpha(float p_alpha){
	alpha = p_alpha;
}
void setEstimacionInicial(int p_estimacion){
	estimacionInicial = p_estimacion;
}
void setAlgoritmo(int p_algoritmo){
	algoritmo = p_algoritmo;
}




/**
 * Incrementa en 1 la unidad de tiempo de ejecucion
 */
void nuevoCicloDeCPU() {
	int i = 0;
	pthread_mutex_lock(&mutexPlanificador);

	for(i = 0; i < list_size(listaEsiListos); i++)
	{
		ESI_STRUCT * esi = list_get(listaEsiListos, i);
		esi->tiempoEspera++;
	}

	for(i = 0; i < list_size(listaEsiBloqueados); i++)
	{
		ESI_STRUCT * esi = list_get(listaEsiBloqueados, i);
		esi->informacionDeBloqueo->unidadesDeTiempoBloqueado++;
	}

	pthread_mutex_unlock(&mutexPlanificador);
}

float calcularMediaExponencial(int duracionRafaga, float estimacionTn) {
	float estimacionTnMasUno = 0.0f;

	//Τn+1 = αtn + (1 - α)Τn
	estimacionTnMasUno = alpha * (float)duracionRafaga + (1.0 - alpha) * estimacionTn;

	return estimacionTnMasUno;
}

int getIndiceMenorEstimacion(){
	int menorIdx = 0, i = 0;
	float menorEstimacion = 100000.0;

	log_info(console_log, "CANTIDAD DE ESIS LISTOS: %d\n", list_size(listaEsiListos));
	log_info(console_log, "id | Tiempo estimado");

	for (i = 0; i < list_size(listaEsiListos); i++) {
		ESI_STRUCT * esi = list_get(listaEsiListos, i);

		if (esi == NULL)
			continue;

		log_info(console_log, "%d | %f", esi->id, esi->tiempoEstimado);

		if(esi->tiempoEstimado < menorEstimacion){
			menorEstimacion = esi->tiempoEstimado;
			menorIdx = i;
		}
	}

	log_info(console_log, "");
	log_info(console_log, "El menor fue el nro %d", menorIdx);

	return menorIdx;
}

int getIndiceMayorResponseRatio() {
	int mayorIdx = 0, i = 0;
	float mayorRR = 0.0;

	log_info(console_log, "CANTIDAD DE ESIS LISTOS: %d\n", list_size(listaEsiListos));

	for (i = 0; i < list_size(listaEsiListos); i++) {
		ESI_STRUCT * esi = list_get(listaEsiListos, i);

		if (esi == NULL)
			continue;

		float rr = calcularTasaDeRespuesta(esi->tiempoEspera, esi->tiempoEstimado);

		log_info(console_log, "N = %d | %d | S: %f | W: %d | RR: %f", i, esi->id, esi->tiempoEstimado, esi->tiempoEspera, rr);

		if(rr > mayorRR){
			mayorRR = rr;
			mayorIdx = i;
		}
	}

	log_info(console_log, "");
	log_info(console_log, "El mayor fue el nro %d", mayorIdx);

	return mayorIdx;
}

void aplicarSJF(bool p_hayDesalojo) {
	hayDesalojo = p_hayDesalojo;

	pthread_mutex_lock(&mutexPlanificador);

	//1 - Si el esi actual termino bloqueado lo encolo
	chequearBloqueoEsiActual();

	//2 - Admitir nuevos ESIs
	admitirNuevosEsis();

	//3 - Se desbloquea algun proceso?
	chequearDesbloqueos();

	//4 - Hay procesos listos para ejecutar?
	if(list_size(listaEsiListos) <= 0)
	{
		log_info(console_log, "Revise, pero no encontre ESIs en estado listo para ejecutar");
		pthread_mutex_unlock(&mutexPlanificador);
		return;
	}

	if(!hayDesalojo){
		if(esiEjecutando == NULL)
		{
			int indexMenorEstimacion = getIndiceMenorEstimacion();
			esiEjecutando = list_get(listaEsiListos, indexMenorEstimacion);
			list_remove(listaEsiListos, indexMenorEstimacion);
		}
	}
	else { //Hay desalojo

		if(esiEjecutando == NULL){
			int indexMenorEstimacion = getIndiceMenorEstimacion();
			esiEjecutando = list_get(listaEsiListos, indexMenorEstimacion);
			list_remove(listaEsiListos, indexMenorEstimacion);
		}
		else {
			int indexMenorEstimacion = getIndiceMenorEstimacion();

			ESI_STRUCT * esiMenorEstimacion = list_get(listaEsiListos, indexMenorEstimacion);

			if(esiMenorEstimacion->id != esiEjecutando->id)
			{
				if(esiMenorEstimacion->tiempoEstimado < esiEjecutando->tiempoEstimado)
				{
					list_add(listaEsiListos, esiEjecutando);
					esiEjecutando = esiMenorEstimacion;
					list_remove(listaEsiListos, indexMenorEstimacion);
				}
			}
		}
	}

	esiEjecutando->tiempoEspera = 0;

	pthread_mutex_unlock(&mutexPlanificador);
}



void aplicarHRRN(){
	pthread_mutex_lock(&mutexPlanificador);

	//1 - Si el esi actual termino bloqueado lo encolo
	chequearBloqueoEsiActual();

	//2 - Admitir nuevos ESIs
	admitirNuevosEsis();

	//3 - Se desbloquea algun proceso?
	chequearDesbloqueos();

	//4 - Hay procesos listos para ejecutar?
	if (list_size(listaEsiListos) <= 0) {
		pthread_mutex_unlock(&mutexPlanificador);
		return;
	}

	//5 - Si no hay esi, planifico el proximo
	if(esiEjecutando == NULL){
		int indexMayorResponseRatio = getIndiceMayorResponseRatio();
		esiEjecutando = list_get(listaEsiListos, indexMayorResponseRatio);
		list_remove(listaEsiListos, indexMayorResponseRatio);

		esiEjecutando->tiempoEspera = 0;
		esiEjecutando->tiempoRafagaActual = 0;
	}

	pthread_mutex_unlock(&mutexPlanificador);
}

/**
 * w = tiempo invertido esperando por el procesador
 * s = tiempo de servicio esperado
 */
float calcularTasaDeRespuesta(int w, float s) {
	if (s == 0.0)
		return 0;

	return (((float)w) + s) / s;
}

void chequearBloqueoEsiActual()
{
	if(esiEjecutando != NULL && esiEjecutando->estado == ESI_BLOQUEADO)
	{
		info_log("El esi actual esta bloqueado, lo mando a la lista de bloqueados");

		esiEjecutando->tiempoEspera = 0;

		// Encolo el esi en la lista de bloqueados
		list_add(listaEsiBloqueados, clonarEsi(esiEjecutando));
		// Libero el esi actual
		esiEjecutando = NULL;
	}
}

void admitirNuevosEsis()
{
	int i = 0;
	if (list_size(listaEsiNuevos) > 0) {
		//1 - Antes de admitirlos les seteo la estimacion inicial
		for (i = 0; i < list_size(listaEsiNuevos); i++) {
			ESI_STRUCT * esi = list_get(listaEsiNuevos, i);

			if (esi == NULL)
				continue;

			esi->tiempoEstimado = (float) estimacionInicial;

			log_info(console_log, "¡Nuevo ESI incorporado! Id asignado: %d", esi->id);
		}

		//2 - Admitir nuevos ESIs
		list_add_all(listaEsiListos, listaEsiNuevos);
		list_clean(listaEsiNuevos);
	}
}

void chequearDesbloqueos(){
	int i, flagLiberar;
	ESI_STRUCT * esi = NULL;

	for(i = 0; i < list_size(listaEsiBloqueados); i++)
	{
		flagLiberar = 0;
		esi = list_get(listaEsiBloqueados, i);

		if(esi == NULL)
			continue;

		ESI_BLOCKED_INFO* infoBloqueo = esi->informacionDeBloqueo;

		if(infoBloqueo == NULL){
			// Si no tiene informacion de bloqueo entonces no esta bloqueado
			flagLiberar = 1;
		}

		if(flagLiberar == 0 && estadoRecurso(infoBloqueo->recursoNecesitado) == RECURSO_LIBRE)
			flagLiberar = 1;

		if(flagLiberar == 1)
		{
			log_info(console_log, "¡Atencion! Se libera un ESI: id %d", esi->id);

			// Calculo la estimacion
			esi->tiempoEstimado = calcularMediaExponencial(esi->tiempoRafagaActual, esi->tiempoEstimado);

			log_info(console_log, "Recalculando estimacion: %f", esi->tiempoEstimado);

			if(algoritmo != 2)
			{
				// Si el algoritmo es SJF ya no me sirve el tiempo de rafaga
				esi->tiempoRafagaActual = 0;
			}

			// Cuando se desbloquea, empieza a contar el tiempo de espera
			esi->tiempoEspera = 0;

			esi->estado = ESI_LISTO;
			strcpy(esi->informacionDeBloqueo->recursoNecesitado, "\0");
			esi->informacionDeBloqueo->unidadesDeTiempoBloqueado = 0;

			// Cambio de lista
			list_remove(listaEsiBloqueados, i);
			i--;
			list_add(listaEsiListos, esi);

			// Ahora hay un esi mas que esta listo para ejecutar
			sem_post(&sem_esis);
		}
	}
}
