#include "server.h"
player players[MAX_PLAYERS];
int amount_players=0;
int running = TRUE;
int stock_addr_size = sizeof(struct sockaddr_in);
int main(int argc , char *argv[])
{
	int server_socket ;
	struct sockaddr_in server_addr, client_addr;
        init_server(&server_socket, &server_addr);
        while(running){
            while(amount_players< MAX_PLAYERS){
                add_player(&players[amount_players], server_socket);
            }
        }
	return 0;
}
void add_player(player * newPlayer, int server_socket){
	int read_size;
        message recvClient;
        message sendClient;
	//Accept and incoming connection
	puts("Waiting for incoming connections...");

	//accept connection from an incoming client
	newPlayer->socket =  accept(server_socket, (struct sockaddr *) &(newPlayer->client_addr), (socklen_t*)&stock_addr_size);
	if (newPlayer->socket < 0)
	{
		perror("accept failed");
		return ;
	}
	printf("Connection accepted\n");

	//Receive a message from client with his name
	while( (read_size = recv(newPlayer->socket , &recvClient, sizeof(message) , 0)) > 0 )
	{
                printf("debug recv: %s\n ",recvClient.payload);
                sendClient.code=C_OK;
                strcpy(sendClient.payload, M_SIGNUP_CLIENT_OK);
		//Send the message back to client
		write(newPlayer->socket ,M_SIGNUP_CLIENT_OK , strlen(M_SIGNUP_CLIENT_OK));
	}

	if(read_size == 0)
	{
		printf("Client disconnected\n");
		fflush(stdout);
	}
	else if(read_size == -1)
	{
		perror("recv failed");
	}

}
void init_server(int *server_socket, struct sockaddr_in *server_addr) {

	if ((*server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error creating the socket");
		exit(EXIT_FAILURE);
	}

	printf("Socket created\n");

	//Prepare the sockaddr_in structure
        server_addr->sin_family = AF_INET;
	server_addr->sin_addr.s_addr = INADDR_ANY;
	server_addr->sin_port = htons(DAMIEN_PORT);

	//Bind
	if( bind(*server_socket,(struct sockaddr *)server_addr , sizeof(*server_addr)) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		exit(EXIT_FAILURE);
	}
        //Listen
	listen(*server_socket , MAX_PLAYERS);
}


