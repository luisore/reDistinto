#ifndef SRC_COORDINADOR_COORDINADOR_H_
#define SRC_COORDINADOR_COORDINADOR_H_

#include <signal.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "../libs/serialize.h"
#include "../libs/tcpserver.h"
#include "../libs/protocols.h"

#include "../configuracion/configuracion.h"
#include "../esi/esi.h"

int conectarseConCoordinador();
void responderCoordinador(int socket, operation_result_e result);
void escucharCoordinador();

void liberarRecursosCoordinador();



#endif /* SRC_COORDINADOR_COORDINADOR_H_ */
