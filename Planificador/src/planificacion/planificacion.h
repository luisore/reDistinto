#ifndef SRC_PLANIFICACION_PLANIFICACION_H_
#define SRC_PLANIFICACION_PLANIFICACION_H_

#include <math.h>
#include "../commons/commons.h"

/********** SJF **********/
float alpha = 0.5f;

int calcularMediaExponencial(int estimacionTn);


/********** HRRN ************/
float calcularTasaDeRespuesta(int w, int s);



#endif /* SRC_PLANIFICACION_PLANIFICACION_H_ */
