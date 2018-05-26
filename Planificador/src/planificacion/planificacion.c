#include "planificacion.h"

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

void aplicarSJF(ESI_STRUCT* esiActual, t_list * listaESIListos, t_list * listaESIBloqueados, bool p_hayDesalojo) {
	hayDesalojo = p_hayDesalojo;
}



void aplicarHRRN(ESI_STRUCT* esiActual, t_list * listaESIListos, t_list * listaESIBloqueados){

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
