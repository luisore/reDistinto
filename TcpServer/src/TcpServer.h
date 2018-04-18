/*
 * TcpServer.h
 *  Handles up to 30 multiple socket connections with Select on Linux.
 *  Created on: 18 abr. 2018
 *      Author: avinocur
 */

#include <stdbool.h>
#include<arpa/inet.h>
#include<commons/log.h>

#ifndef TCPSERVER_H_
#define TCPSERVER_H_


typedef struct {
	char* name;
	int master_socket;
	bool listen_console;
	struct sockaddr_in* address;
	int* client_sockets;
	int max_clients;
	t_log* logger;

} tcp_server_t;

tcp_server_t * tcpserver_create(char* server_name, char* log_filename, int max_clients,
		int connection_queue_size, int port, bool listen_console);

void tcpserver_destroy(tcp_server_t* server);

int tcpserver_run(tcp_server_t* server,
		void (*on_accept)(tcp_server_t*, int),
		void (*on_read)(tcp_server_t*, int, int),
		void (*on_command)(tcp_server_t*));

void tcpserver_remove_client(tcp_server_t* server, int socket_id);

#endif /* TCPSERVER_H_ */
