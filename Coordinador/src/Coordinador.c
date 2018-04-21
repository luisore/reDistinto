#include "Coordinador.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
//int readConfig(char* configFile) {
//	if (configFile == NULL) {
//		return -1;
//	}
//	t_config *config = config_create(configFile);
//	log_info(console_log, " .:: Cargando settings ::.");
//
//	if (config != NULL) {
//		coordinador_setup.PUERTO_ESCUCHA_CONEXIONES = config_get_int_value(
//				config, "PUERTO_ESCUCHA_CONEXIONES");
//		coordinador_setup.ALGORITMO_DISTRIBUCION = config_get_int_value(config,
//				"ALGORITMO_DISTRIBUCION");
//		coordinador_setup.CANTIDAD_ENTRADAS = config_get_int_value(config,
//				"CANTIDAD_ENTRADAS");
//		coordinador_setup.TAMANIO_ENTRADA_BYTES = config_get_int_value(config,
//				"TAMANIO_ENTRADA_BYTES");
//		coordinador_setup.RETARDO_MS = config_get_int_value(config,
//				"RETARDO_MS");
//	}
//	config_destroy(config);
//	return 0;
//}

int main(void) {
	struct sockaddr_in servidorConfig;
	servidorConfig.sin_family = AF_INET;
	servidorConfig.sin_addr.s_addr = INADDR_ANY;
	servidorConfig.sin_port = htons(8080);

		int servidor = socket(AF_INET, SOCK_STREAM, 0);

		int activado = 1;//Se usaba para una funcion que supuestamente liberaba el puerto para que sea reutilizado. Pero no funcionaba.

		if (bind(servidor, (void*) &servidorConfig, sizeof(servidorConfig)) != 0) {
			perror("Falló el bind");
			return 1;
		}

		printf("Estoy escuchando\n");
		listen(servidor, 100);

		//------------------------------
		struct sockaddr_in clienteConfig;
		unsigned int direccion;
		int cliente = accept(servidor, (void*) &clienteConfig, &direccion);
		if(cliente < 0){
			perror("Error al aceptar el cliente");
			printf("El acept retorno %d\n", cliente);
			return 1;
		}
		printf("Recibí una conexión en %d!!\n", cliente);
		send(cliente, "Hola NetCat!", 13, 0);
		send(cliente, ":)\n", 4, 0);

		//------------------------------

		char* bufferServidor = malloc(50);
//
		while (1) {
			int bytesRecibidos = recv(cliente, bufferServidor, 1000, 0);
			if (bytesRecibidos <= 0) {
				perror("El chabón se desconectó o bla.");
				return 1;
			}

			bufferServidor[bytesRecibidos] = '\0';
			printf("Me llegaron %d bytes con %s\n", bytesRecibidos, bufferServidor);
		}
//
		free(bufferServidor);

		return 0;
	}
//	console_log = log_create("coordinador.log", "ReDistinto-Coordinador",
//	true, LOG_LEVEL_TRACE);
//	printf("\n\t\e[31;1m=========================================\e[0m\n");
//	printf("\t.:: Bievenido a ReDistinto ::.");
//	printf("\n\t\e[31;1m=========================================\e[0m\n\n");
//	if (readConfig(PATH_FILE_NAME) < 0) {
//		log_error(console_log, "No se encontró el archivo de configuración");
//		return -1;
//	}
//	log_info(console_log, "Se cargó el setup del COORDINADOR");
//
//	log_info(console_log, "");
//
//	log_info(console_log, "\tPuerto conecciones: %d",
//			coordinador_setup.PUERTO_ESCUCHA_CONEXIONES);
//
//	switch (coordinador_setup.ALGORITMO_DISTRIBUCION) {
//	case LSU:
//		log_info(console_log, "\tAlgoritmo de distribucion: LSU");
//		break;
//	case EL:
//		log_info(console_log, "\tAlgoritmo de distribucion: EL");
//		break;
//	case KE:
//		log_info(console_log, "\tAlgoritmo de distribucion: KE");
//		break;
//	}
//
//	log_info(console_log, "\tCantidad de entradas: %d",
//			coordinador_setup.CANTIDAD_ENTRADAS);
//
//	log_info(console_log, "\tTamanio de entrada en bytes: %d",
//			coordinador_setup.TAMANIO_ENTRADA_BYTES);
//
//	log_info(console_log, "\tRetardo en milis: %d",
//			coordinador_setup.RETARDO_MS);
//
//	log_info(console_log, "Se cerro la conexion con el planificador");
//	printf("\n\t\e[31;1m Consola terminada. \e[0m\n");
//
//	log_destroy(console_log);
