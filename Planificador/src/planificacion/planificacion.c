#include "planificacion.h"

int calcularMediaExponencial(int estimacionTn){
	float estimacionTnMasUno = 0.0f;

	//Τn+1 = αtn + (1 - α)Τn
	estimacionTnMasUno = alpha * estimacionTn + (1 - alpha) * estimacionTn;

	return (int) roundf(estimacionTnMasUno);
}

/**
 * w = tiempo invertido esperando por el procesador
 * s = tiempo de servicio esperado
 */
float calcularTasaDeRespuesta(int w, int s) {
	return (w + s) / s;
}
