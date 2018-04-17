/*
 * TcpServer.h
 *
 *  Handles multiple socket connections with Select on Linux
 *  Created on: 18 abr. 2018
 *      Author: avinocur
 */

#include "tcpserver.h"
#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

#include <signal.h>
#include <commons/string.h>


tcp_server_t* tcpserver_create(char* server_name, t_log* log, int max_clients, int connection_queue_size, int port, bool listen_console){

	tcp_server_t* server = malloc(sizeof(tcp_server_t));
	server->name = string_duplicate(server_name);
	server->client_sockets = malloc(sizeof(int)*max_clients);
	server->logger = log;
	server->listen_console = listen_console;
	server->address = malloc(sizeof(struct sockaddr_in));
	server->max_clients = max_clients;
	server->master_socket = 0;

	log_info(server->logger, "Initializing TCP server: %s on port %d.", server_name, port);

	int opt = true;


	//initialize all client_socket[] to 0 so not checked
	int i;
	for (i = 0; i < max_clients; i++)
	{
		server->client_sockets[i] = 0;
	}

	//create a master socket
	if( (server->master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
	{
		log_error(server->logger, "Could not create master socket for server: %s.", server_name);
		tcpserver_destroy(server);
		return NULL;
	}

	//set master socket to allow multiple connections , this is just a good habit, it will work without this
	if( setsockopt(server->master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
	{
		log_error(server->logger, "Could not configure server %s for multiple connections on master socket.", server_name);
		tcpserver_destroy(server);
		return NULL;
	}
	log_info(server->logger, "TCP Server %s created on socket: %d", server_name, server->master_socket);

	//type of socket created
	server->address->sin_family = AF_INET;
	server->address->sin_addr.s_addr = INADDR_ANY;
	server->address->sin_port = htons( port );

	//bind the socket to localhost on specified port
	if (bind(server->master_socket, (struct sockaddr *)(server->address), sizeof(struct sockaddr_in))<0)
	{
		log_error(server->logger, "Could not bind TCP Server %s to port %d.", port, server_name);
		tcpserver_destroy(server);
		return NULL;
	}
	log_info(server->logger, "TCP Server %s bound to port: %d.", server_name, port);

	//try to specify maximum of 3 pending connections for the master socket
	if (listen(server->master_socket, connection_queue_size) < 0)
	{
		log_error(server->logger, "Could not listen on socket.");
		tcpserver_destroy(server);
		return NULL;
	}

	log_info(server->logger, "TCP Server %s successfully created", server_name);

	return server;
}

void tcpserver_destroy(tcp_server_t* server){
	free(server->name);
	close(server->master_socket);
	free(server->address);
	free(server->client_sockets);

	free(server);
}

bool tcpserver_accept_new_connection(tcp_server_t* server, void (*on_accept)(tcp_server_t*, int, int)){
	int new_socket;
	int addrlen = sizeof(server->address);

	if ((new_socket = accept(server->master_socket, (struct sockaddr *)(server->address), (socklen_t*)&addrlen))<0){
		log_error(server->logger, "Could not accept connection on socket for server: %s", server->name);
		return false;
	}

	//inform user of socket number - used in send and receive commands
	log_info(server->logger, "New connection received on TCP Server: %s. Socket: %d, IP:: %s, Port: %d.",
			server->name, new_socket , inet_ntoa(server->address->sin_addr) , ntohs(server->address->sin_port));

	//add new socket to array of sockets
	int i;
	for (i = 0; i < server->max_clients; i++)
	{
		//if position is empty
		if( server->client_sockets[i] == 0 )
		{
			server->client_sockets[i] = new_socket;
			log_info(server->logger, "TCPServer: %s. Added new connection to list of sockets as: %d.", server->name, i);
			on_accept(server, new_socket, i);
			break;
		}
	}

	log_info(server->logger, "Connection on TCP Server: %s received successfully.", server->name);

	return true;
}

void tcpserver_handle_reads(tcp_server_t* server, fd_set* readfds,
		void (*on_read)(tcp_server_t*, int, int),
		void (*on_command)(tcp_server_t*)){
	int i;
	int client_sd;
	for (i = 0; i < server->max_clients; i++){
		client_sd = server->client_sockets[i];

		if (FD_ISSET( client_sd , readfds)){
			if(client_sd == STDIN_FILENO){
				on_command(server);
			} else {
				on_read(server, client_sd, i);
			}
		}
	}
}

int tcpserver_run(tcp_server_t* server,
		void (*before_cycle)(tcp_server_t*),
		void (*on_accept)(tcp_server_t*, int, int),
		void (*on_read)(tcp_server_t*, int, int),
		void (*on_command)(tcp_server_t*))
{
	fd_set readfds, errorfds;

	log_info(server->logger, "TCP Server %s waiting for connections ...", server->name);

    int sd;
    int max_sd;
    int activity;

    while(true){
    	if(before_cycle != NULL){
    		before_cycle(server);
    	}

    	//clear the socket set
        FD_ZERO(&readfds);
        FD_ZERO(&errorfds);

        //add master socket to set
        FD_SET(server->master_socket, &readfds);
        //FD_SET(server->master_socket, &errorfds);

        max_sd = server->master_socket;

        if(server->listen_console){
        	FD_SET(STDIN_FILENO, &readfds);
        }

        //add child sockets to set
        int i;
        for ( i = 0; i < server->max_clients ; i++){
            sd = server->client_sockets[i];

            //if it's a valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET( sd , &readfds);

            // keep the highest file descriptor number, needed for the select function
            if(sd > max_sd)
                max_sd = sd;
        }

        // Wait for an activity on one of the sockets. Timeout is NULL, so wait indefinitely
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

        if ((activity < 0) && (errno!=EINTR)){
            log_error(server->logger, "Error on select for server: %s", server->name);
        	return EXIT_FAILURE;
        }

        // If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(server->master_socket, &readfds))
        {
        	// Accept the incoming connection
            if(!tcpserver_accept_new_connection(server, on_accept)){
            	return EXIT_FAILURE;
            }
        }

        //else its some IO operation on some other socket :)
        tcpserver_handle_reads(server, &readfds, on_read, on_command);
    }

    return EXIT_SUCCESS;
}

void tcpserver_remove_client(tcp_server_t* server, int socket_id){
	close( server->client_sockets[socket_id]);
	server->client_sockets[socket_id] = 0;
}
