/**
*
*  AUTHORS : MANIET Alexandre (amaniet15)(serie 2) , MEUR Damien (dmeur15)(serie 2)
*  This file contains all the  implemented functions needed by both the server and the player program.
*
*/

#include "socket.h"
void init_server(int *server_socket, struct sockaddr_in *server_addr, int port) {

	if ((*server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error creating the socket");
		exit(EXIT_FAILURE);
	}
	//Prepare the sockaddr_in structure
        server_addr->sin_family = AF_INET;
	server_addr->sin_addr.s_addr = INADDR_ANY;
	server_addr->sin_port = htons(port);

	//Bind
	if( bind(*server_socket,(struct sockaddr *)server_addr , sizeof(*server_addr)) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		exit(EXIT_FAILURE);
	}
        //Listen
        listen(*server_socket , 10);
        printf("Socket created and binded on port %i\n", port);
}

void connect_to_server(int *client_socket , struct sockaddr_in *server_addr, char * server_ip, int port){
	//Create socket
	*client_socket = socket(AF_INET , SOCK_STREAM , 0);
	if (*client_socket == -1)
	{
		printf("Could not create socket");
	}
	server_addr->sin_addr.s_addr = inet_addr(server_ip);
	server_addr->sin_family = AF_INET;
	server_addr->sin_port = htons(port );

	//Connect to remote server
	if (connect(*client_socket , (struct sockaddr *)server_addr , sizeof(*server_addr)) < 0)
	{
		perror("connect failed. Error");
		exit(EXIT_FAILURE);
	}
	printf("Connected to server\n");


}



int receive_message(message* msg, int fd) {
	int bytes_received;
	if ((bytes_received = recv(fd, msg, sizeof( message), 0)) <= 0) {
		if (bytes_received == 0) {
			fprintf(stderr, "Client disconnected.\n");
		}
		else {
			perror("Could not receive message");
		}
		return FALSE;
	}
	return TRUE;
}
void send_message(message msg, int socket) {
    if (send(socket, &msg, sizeof(struct message), 0) == -1) {
        perror("Failed to send a message to the socket");
    }
}
void shutdown_socket(int socket) {
	if (close(socket) < 0) {
		perror("Socket shutdown");
		exit(EXIT_FAILURE);
	}
}

