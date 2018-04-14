#ifndef ESI_H_
#define ESI_H_

/*MACROS*/
#define PATH_FILE_NAME "esi.config"

/*VARIABLES GLOBALES*/
t_log *console_log;

/*ESTRUCTURAS*/
struct {
	char* IP_COORDINADOR;
	int PUERTO_COORDINADOR;
	char* IP_PLANIFICADOR;
	int PUERTO_PLANIFICADOR;
} esi_setup;

/*FUNCIONES*/
int readConfig(char* configFile);

#endif /* ESI_H_ */
