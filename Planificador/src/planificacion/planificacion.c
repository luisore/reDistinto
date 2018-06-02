#include "planificacion.h"
#include "../esi/esi.h"

// PRIVADAS
bool hayDesalojo = false;
float alpha = 0.5f;
int estimacionInicial = 0;

void chequearDesbloqueos();


// SETTERS
void setAlpha(float p_alpha){
	alpha = p_alpha;
}
void setEstimacionInicial(int p_estimacion){
	estimacionInicial = p_estimacion;
}




/**
 * Incrementa en 1 la unidad de tiempo de ejecucion
 */
void nuevoCicloDeCPU() {
	int i = 0;
	pthread_mutex_lock(&mutexPrincipal);

	for(i = 0; i < list_size(listaEsiListos); i++)
	{
		ESI_STRUCT * esi = list_get(listaEsiListos, i);
		esi->tiempoEspera++;
	}

	for(i = 0; i < list_size(listaEsiBloqueados); i++)
	{
		ESI_STRUCT * esi = list_get(listaEsiBloqueados, i);
		esi->tiempoEspera++;
		esi->informacionDeBloqueo->unidadesDeTiempoBloqueado++;
	}

	pthread_mutex_unlock(&mutexPrincipal);
}




int calcularMediaExponencial(int duracionRafaga, int estimacionTn) {
	float estimacionTnMasUno = 0.0f;

	//Τn+1 = αtn + (1 - α)Τn
	estimacionTnMasUno = alpha * duracionRafaga + (1 - alpha) * estimacionTn;

	return (int) roundf(estimacionTnMasUno);
}

int getIndiceMenorEstimacion(){
	int menorIdx = 0, menorEstimacion = 100000, i = 0;

	log_info(console_log, "\nCANTIDAD DE ESIS LISTOS: %d\n", list_size(listaEsiListos));

	for (i = 0; i < list_size(listaEsiListos); i++) {
		ESI_STRUCT * esi = list_get(listaEsiListos, i);

		if (esi == NULL)
			continue;

		if(esi->tiempoEstimado < menorEstimacion){
			menorEstimacion = esi->tiempoEstimado;
			menorIdx = i;
		}
	}
	return menorIdx;
}

void aplicarSJF(bool p_hayDesalojo) {
	int i = 0;
	hayDesalojo = p_hayDesalojo;

	pthread_mutex_lock(&mutexPrincipal);

	// 0 - Si el esi actual termino bloqueado lo encolo
	if(esiEjecutando != NULL && esiEjecutando->estado == ESI_BLOQUEADO)
	{
		log_info(console_log, "El esi actual esta bloqueado, lo mando a la lista de bloqueados");
		// Encolo el esi en la lista de bloqueados
		list_add(listaEsiBloqueados, clonarEsi(esiEjecutando));
		// Libero el esi actual
		esiEjecutando = NULL;
	}

	if(list_size(listaEsiNuevos) > 0)
	{
		//1 - Antes de admitirlos les seteo la estimacion inicial
		for (i = 0; i < list_size(listaEsiNuevos); i++) {
			ESI_STRUCT * esi = list_get(listaEsiNuevos, i);

			if (esi == NULL)
				continue;

			esi->tiempoEstimado = estimacionInicial;
		}

		//2 - Admitir nuevos ESIs
		list_add_all(listaEsiListos, listaEsiNuevos);
		list_clean(listaEsiNuevos);
	}

	//3 - Se desbloquea algun proceso?
	chequearDesbloqueos();

	//4 - Hay procesos listos para ejecutar?
	if(list_size(listaEsiListos) <= 0)
	{
		pthread_mutex_unlock(&mutexPrincipal);
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
	pthread_mutex_unlock(&mutexPrincipal);
}



void aplicarHRRN(){
	// 1) Chequear bloqueados: alguno se libero? -> Por c/u incrementar contador de tiempo y de tiempo de bloqueo.
	// 2) Incrementar contador de tiempo para cada ESI listo
	// 3) Calcular tasa de respuesta por cada esi listo
	// 4) Elegir esi actual
}

/**
 * w = tiempo invertido esperando por el procesador
 * s = tiempo de servicio esperado
 */
float calcularTasaDeRespuesta(int w, int s) {
	if (s == 0)
		return 0;

	return (w + s) / s;
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

		if(estadoRecurso(infoBloqueo->recursoNecesitado) == RECURSO_LIBRE)
			flagLiberar = 1;

		if(flagLiberar == 1)
		{
			// Calculo la estimacion
			esi->tiempoEstimado = calcularMediaExponencial(esi->tiempoRafagaActual, esi->tiempoEstimado);

			// Cambio de lista
			list_remove(listaEsiBloqueados, i);
			i--;
			list_add(listaEsiListos, esi);
		}
	}
}
