#include "planificacion.h"
#include "../esi/esi.h"

// VARIABLES PRIVADAS
bool hayDesalojo = false;
float alpha = 0.5f;

void setAlpha(float p_alpha){
	alpha = p_alpha;
}

int calcularMediaExponencial(int estimacionTn) {
	float estimacionTnMasUno = 0.0f;

	//Τn+1 = αtn + (1 - α)Τn
	estimacionTnMasUno = alpha * estimacionTn + (1 - alpha) * estimacionTn;

	return (int) roundf(estimacionTnMasUno);
}

void aplicarSJF(bool p_hayDesalojo) {
	hayDesalojo = p_hayDesalojo;

	// 1) Chequear bloqueados: alguno se libero? -> Por c/u incrementar contador de tiempo y de tiempo de bloqueo.
	// 2) Incrementar contador de tiempo para cada ESI listo
	// 3) Calcular media exponencial por cada esi listo
	// 4) Si hay desalojo aplicarlo
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
