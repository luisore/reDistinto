/*
 * TcpServer.h
 *  Handles up to 30 multiple socket connections with Select on Linux.
 *  Created on: 18 abr. 2018
 *      Author: avinocur
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <commons/log.h>
#include <commons/string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <netinet/in.h>
#include <signal.h>

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

/*
 * TCP Server will not destroy the supplied t_log.
 * You must create and destroy it on your own.
 */
tcp_server_t * tcpserver_create(char* server_name, t_log* logger,
		int max_clients, int connection_queue_size, int port,
		bool listen_console);

void tcpserver_destroy(tcp_server_t* server);

int tcpserver_run(tcp_server_t* server, void (*before_cycle)(tcp_server_t*),
		void (*on_accept)(tcp_server_t*, int, int),
		void (*on_read)(tcp_server_t*, int, int),
		void (*on_command)(tcp_server_t*));

void tcpserver_remove_client(tcp_server_t* server, int socket_id);

#endif /* TCPSERVER_H_ */
