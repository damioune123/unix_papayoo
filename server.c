#include "server.h"
int main(int argc , char *argv[])
{
	int socket_desc , client_sock , c , read_size;
	struct sockaddr_in server , client;
	player players[MAX_PLAYERS];
	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
	puts("Socket created");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 8888 );

	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		return 1;
	}
	puts("bind done");

	//Listen
	listen(socket_desc , 3);

	while(amount_players < MAX_PLAYERS){
		signup_daemon();
	}
	return 0;
}
void signup_daemon(){
	player newPlayer;
	int read_size;
	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);

	//accept connection from an incoming client
	newPlayer.socket =  accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
	if (newPlayer.socket < 0)
	{
		perror("accept failed");
		return 1;
	}
	puts("Connection accepted");

	//Receive a message from client
	while( (read_size = recv(newPlayer.socket , newSocket.name , 255 , 0)) > 0 )
	{
		//Send the message back to client
		write(newPlayer.socket , SIGNUP_CLIENT_OK , strlen(SIGNUP_CLIENT_OK));
	}

	if(read_size == 0)
	{
		puts("Client disconnected");
		fflush(stdout);
	}
	else if(read_size == -1)
	{
		perror("recv failed");
	}

	return 0;
}







