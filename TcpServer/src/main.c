/*
 * main.c
 *
 *  Created on: 18 abr. 2018
 *      Author: utnso
 */
#include "TcpServer.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <commons/string.h>

void on_accept(tcp_server_t* server, int client_socket);
void on_read(tcp_server_t* server, int client_socket, int socket_id);
void on_command(tcp_server_t* server);

int main(){
	tcp_server_t* server = tcpserver_create("TEST_SERVER", "tcpserver.log", 10, 3, 8080, true);

	return tcpserver_run(server, on_accept, on_read, on_command);
}

void on_accept(tcp_server_t* server, int client_socket){
	//send new connection greeting message
	char* greeting = "Welcome to the TCP Server!\r\n";
	if( send(client_socket, greeting, strlen(greeting), 0) != strlen(greeting) )
	{
		log_error(server->logger, "Could not send message to client");
	}
}

void on_read(tcp_server_t* server, int client_socket, int socket_id){
	int valread;
	char buffer[1024];
	int addrlen = sizeof(struct sockaddr_in);
	//Check if it was for closing , and also read the incoming message
	if ((valread = read( client_socket, buffer, 1024)) == 0) {
		//Somebody disconnected , get his details and print
		getpeername(client_socket, (struct sockaddr*)server->address , (socklen_t*)&addrlen);
		log_info(server->logger, "TCP Server: %s Host disconnected , IP:%s , PORT: %d \n" ,
				server->name, inet_ntoa(server->address->sin_addr) , ntohs(server->address->sin_port));

		//Close the socket and mark as 0 in list for reuse
		tcpserver_remove_client(server, socket_id);
	}

	//Echo back the message that came in
	else
	{	//set the string terminating NULL byte on the end of the data read
		buffer[valread] = '\0';
		send(client_socket , buffer , strlen(buffer) , 0 );
	}
}

void on_command(tcp_server_t* server){
	int valread;
	char buffer[1024];

	valread = read(STDIN_FILENO, buffer, 1024);

	// To skip the \n...
	buffer[valread-1] = '\0';

	if(strcmp("exit", buffer) == 0){
		printf("Exit command received.\n");
		log_info(server->logger, "TCP Server %s. Exit requested by console.", server->name);
		tcpserver_destroy(server);
		exit(0);
	} else {
		printf("Unknown command: %s. Enter 'exit' to exit.\n", buffer);
	}
}
