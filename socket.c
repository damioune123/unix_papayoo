/**
*
*  AUTHORS : MANIET Alexandre (amaniet15)(serie 2) , MEUR Damien (dmeur15)(serie 2)
*  This file contains all the  implemented functions needed by both the server and the player program.
*
*/

#include "socket.h"
/**
 *
 * This function ought to be used by server side only. It aims to init the server by creating a socket, binding it with the sockaddr_in initialized to use a certain port, the current ip_addrress of the machin and the TCP protocol.
 * The socket will be then listen to any connection to the server socket with a certain maximum pending connections.
 *
 * @param int *server_socket: the empty file descriptor of the server socket, it will be initialized in this function.
 *
 * @param struct sockaddr_in *server_addr : the empty sockaddr_in structure that will be initialized and binded with server socket in this function.
 *
 * @param int max_pending_connections : the amount of max pending connections to the server socket, by security it ough to be the MAX_PLAYERS const (passed in parameter here to make the code reusable)
 *
 */
void init_server(int *server_socket, struct sockaddr_in *server_addr, int port, int max_pending_connections) {

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
        listen(*server_socket , max_pending_connections);
        printf("Socket created and binded on port %i\n", port);
}
/**
 *
 * This function ought to be used by clent side only. It aims to link a socket file descriptor with a remote server socket, that is to say connect a client to the server.
 * 
 * @param int * client_socket : the empty file descriptor of the socket client side to connect with the server side's one.
 *
 * @param struct sockaddr_in *server_addr : the empty sockaddr_in that will be initialized with server information (IP address , port and protocol (here TCP))
 *
 * @param char * server_ip : the ip addres of the remote server
 *
 * @param int port : the port of the socket of the remote server
 *
 *
 */
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
		perror("No connection could be established with the remote server\n");
		exit(EXIT_FAILURE);
	}
	printf("Connected to server\n");
}
/**
 *
 * This function aims to receive a message structure (see message.h) through a connected socket.
 *
 * @param message *mesg : a pointer to a message structure that is going to be filled with the information contained in the received message structure.
 *
 * @param int fd : the connected socket file descriptor  to receive a message from.
 *
 *
 */
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
/**
 * This function aims to send a message (see message.h) to a connected socket.
 *
 * @param message msg : the message structure with the information to send.
 *
 * @int socket : the connected socket file descriptor to send message to.
 *
 *
 */
void send_message(message msg, int socket) {
    if (send(socket, &msg, sizeof(struct message), 0) == -1) {
        perror("Failed to send a message to the socket");
    }
}
/**
 * This function close a socket using its file descriptor.
 *
 * @param int socket : the socket file descriptor
 *
 *
 */
void shutdown_socket(int socket) {
	if (close(socket) < 0) {
		perror("Socket shutdown");
		exit(EXIT_FAILURE);
	}
}

