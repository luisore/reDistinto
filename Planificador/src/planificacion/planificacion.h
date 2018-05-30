#ifndef SRC_PLANIFICACION_PLANIFICACION_H_
#define SRC_PLANIFICACION_PLANIFICACION_H_

#include <math.h>
#include "../commons/commons.h"


/********** SJF **********/
void setAlpha(float p_alpha);
void setEstimacionInicial(int p_estimacion);
int calcularMediaExponencial(int duracionRafaga, int estimacionTn);
void aplicarSJF(bool p_hayDesalojo);


/********** HRRN ************/
float calcularTasaDeRespuesta(int w, int s);
void aplicarHRRN();


#endif /* SRC_PLANIFICACION_PLANIFICACION_H_ */
