#ifndef SRC_PLANIFICACION_PLANIFICACION_H_
#define SRC_PLANIFICACION_PLANIFICACION_H_

#include <math.h>
#include "../commons/commons.h"


/**
 * Incrementa en 1 la unidad de tiempo de ejecucion
 */
void nuevoCicloDeCPU();

void setAlgoritmo(int p_algoritmo);

/********** SJF **********/
void setAlpha(float p_alpha);
void setEstimacionInicial(int p_estimacion);
float calcularMediaExponencial(int duracionRafaga, float estimacionTn);
void aplicarSJF(bool p_hayDesalojo);


/********** HRRN ************/
float calcularTasaDeRespuesta(int w, int s);
void aplicarHRRN();


void chequearBloqueoEsiActual();


#endif /* SRC_PLANIFICACION_PLANIFICACION_H_ */
