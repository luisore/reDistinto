#include "consola_instancia.h"
#include "libs/protocols.h"
#include <stdlib.h>
#include <stdlib.h>


void print_header() {
	printf("\n\t\e[31;1m=========================================\e[0m\n");
	printf("\t.:: CONSOLA PARA INSTANCIA ::.");
	printf("\n\t\e[31;1m=========================================\e[0m\n\n");
}

void print_goodbye() {
	printf("\n\t\e[31;1m=========================================\e[0m\n");
	printf("\t.:: FIN EJECUCION::.");
	printf("\n\t\e[31;1m=========================================\e[0m\n\n");
}

void exit_program(int entero) {

	printf("\n\t\e[31;1m FINALIZA CONSOLA \e[0m\n");
	exit(entero);
}

void _finalizar_cadena(char *entrada)
{
	if ((strlen(entrada) > 0) && (entrada[strlen (entrada) - 1] == '\n'))
			entrada[strlen (entrada) - 1] = '\0';
}

char* _obtener_comando(char** split_comandos)
{
	return split_comandos[0];
}

char* _obtener_primer_parametro(char** split_comandos)
{
	return split_comandos[1];
}

char* _obtener_segundo_parametro(char** split_comandos)
{
	return split_comandos[2];
}

void operacion_get(char * clave){
	printf("\n\t OPERACION GET - Clave: %s   \n" , clave);
//	send_operation(socketcito , idecito);
//	send_get_operation(socketcito , idecito);
}

void operacion_set(char * parametro1 , char * parametro2){
	if(send_operation()){
		send_set_operation(parametro1 , parametro2);

	}

}

void operacion_store(char * clave){
	printf("\n\t OPERACION STORE - Clave: %s   \n" , clave);
}


void input(){

	printf(".");

	size_t size = 20;
	char *entrada = malloc(20);

	char *comando = malloc(sizeof(char*));
	char *parametro1 = malloc(sizeof(char*));
	char *parametro2 = malloc(sizeof(char*));
	char** split_comandos = malloc(sizeof(char**));

	fgets(entrada, size, stdin);

	_finalizar_cadena(entrada);
	split_comandos = string_split(entrada, " ");

	comando = _obtener_comando(split_comandos);

	if ( !strcmp(comando ,"GET") ){
		parametro1 = _obtener_primer_parametro(split_comandos);
		operacion_get(parametro1);
	}
	if (!strcmp(comando ,"SET")){
		parametro1 = _obtener_primer_parametro(split_comandos);
		parametro2 = _obtener_segundo_parametro(split_comandos);
		operacion_set(parametro1 , parametro2);
	}
	if (!strcmp(comando ,"STORE")){
		parametro1 = _obtener_primer_parametro(split_comandos);
		operacion_store(parametro1);
	}

}

void send_message_instance(t_connection_header *connection_header, int client_socket, int socket_id){
	t_instance_init_values init_values_message;
			init_values_message.entry_size = 10;
			init_values_message.number_of_entries = 3;
			void *init_value_instance_buffer = serialize_init_instancia_message(&init_values_message);

			if( send(client_socket, init_value_instance_buffer, INSTANCE_INIT_VALUES_SIZE, 0) != INSTANCE_INIT_VALUES_SIZE)
			{
				//todo
			} else {
			}
			free(init_value_instance_buffer);
}


void send_set_operation(char * clave , char * valor){

	t_operation_request operacion;

	strcpy(operacion.key , clave);
	operacion.operation_type = SET;
	operacion.payload_size = strlen(valor);

	void *buffer  = serialize_operation_request(&operacion);

	if( send(instance_socket, buffer, OPERATION_REQUEST_SIZE, 0) != OPERATION_REQUEST_SIZE)
	{

	} else {

		if(send(instance_socket, valor, strlen(valor), 0)){
			printf("se envio correctamente");

		}

	}
	free(buffer);



}


int send_operation(){

	t_coordinator_operation_header header;

	header.coordinator_operation_type = KEY_OPERATION;

	void *init_value_instance_buffer = serialize_coordinator_operation_header(&header);

	if( send(instance_socket, init_value_instance_buffer, COORDINATOR_OPERATION_HEADER_SIZE, 0) != COORDINATOR_OPERATION_HEADER_SIZE)
	{
		return 0;
	} else {
		return 1;
	}

	free(init_value_instance_buffer);

}

void send_get_operation(int client_socket, int socket_id , char * clave ){

//	t_operation_request request;
//
//	strcpy(request.key, clave);
//	request.operation_type = SET;
//	request.payload_size = ""
//
//	void * buffer = serialize_operation_request(&request);
//
//	if( send(client_socket, buffer, OPERATION_REQUEST_SIZE, 0) != OPERATION_REQUEST_SIZE)
//	{
//		// TODO
//	} else {
//	}
//	free(buffer);
//


}

void aceptar_conexion(int client_socket, int socket_id){
	void *header_buffer = malloc(CONNECTION_HEADER_SIZE);

	int res = recv(client_socket, header_buffer, CONNECTION_HEADER_SIZE, MSG_WAITALL);
	if (res <= 0) {
		printf("fallo");
		free(header_buffer);
		return;
	}

	t_connection_header *connection_header = deserialize_connection_header(header_buffer);
	free(header_buffer);
	switch (connection_header->instance_type){
	case REDIS_INSTANCE:
		send_message_instance(connection_header, client_socket, socket_id);
		break;
	}

	free(connection_header);
}

void leer_pedido(int client_socket){

	printf("Estoy por enviar");

	void *package_buffer = malloc(INSTANCE_RESPONSE_SIZE);

	if (recv(client_socket, package_buffer, INSTANCE_RESPONSE_SIZE, MSG_WAITALL) < INSTANCE_RESPONSE_SIZE) {
		free(package_buffer);
		return;
	}

	t_instance_response * response = deserialize_instance_response(package_buffer);

	printf("LLEGO EL PEDIDO");

	printf("RESPONSE: %s" , response->status , response->payload_size);

	free(package_buffer);
	free(response);

}


void leer_consola(){

	while (true) {
		input();
	}
}

void conectarse(){

	struct sockaddr_in servidorConfig;
	servidorConfig.sin_family = AF_INET;
	servidorConfig.sin_addr.s_addr = INADDR_ANY;
	servidorConfig.sin_port = htons(8000);

	int servidor = socket(AF_INET, SOCK_STREAM, 0);

	if (bind(servidor, (struct sockaddr *) &servidorConfig, sizeof(struct sockaddr_in)) != 0) {
		perror("Falló el bind");
		exit_program(EXIT_FAILURE);
	}

	printf("Estoy escuchando\n");
	listen(servidor, 100);

	int addrlen = sizeof(&servidorConfig);

	unsigned int direccion;
	int cliente = accept(servidor, (struct sockaddr *) &servidorConfig, (socklen_t*)&addrlen);
	if(cliente < 0){
		perror("Error al aceptar el cliente");
		printf("El acept retorno %d\n", cliente);
		exit_program(EXIT_FAILURE);
	}
	printf("Recibí una conexión en %d!!\n", cliente);

	instance_socket = cliente;

	aceptar_conexion(cliente,0);

}


int main(void) {

	print_header();

	conectarse();

	leer_consola();

	print_goodbye();
	exit_program(EXIT_SUCCESS);
	return 0;
}
