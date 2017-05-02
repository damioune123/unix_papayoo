#include "server.h"
player players[MAX_PLAYERS];
int port;
FILE *fpError;
int amount_players=0;
int running = TRUE;
int stock_addr_size = sizeof(struct sockaddr_in);
int main(int argc , char *argv[])
{
        int fdLock = open("./server.lock", O_RDWR);
	if (fdLock == -1) { 
		perror("Erreur ouverture fichier lock\n");
		exit(EXIT_FAILURE);
	}
	if (flock(fdLock, LOCK_EX | LOCK_NB) == -1) { 
		fprintf(stderr,"Ce daemon ne peut pas etre ouvert plusieurs  fois, une autre instance est en cours\n");
		exit(EXIT_FAILURE);
	}
        if(argc!=2 && argc !=3){
            fprintf(stderr, "Usage %s port [log_file]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        if(argc==3)
            fpError =freopen(argv[2], "a", stderr);
        fprintf(stderr, "test\n");
	int server_socket ;
	struct sockaddr_in server_addr, client_addr;
        init_server(&server_socket, &server_addr);
        while(running){
            while(amount_players< MAX_PLAYERS){
                add_player(&players[amount_players], server_socket);
            }
        }
        if(argc==3)
            fclose(fpError);
	return 0;
}
void add_player(player * newPlayer, int server_socket){
	int read_size;
        message recvClient;
        message sendClient;
	//Accept and incoming connection
	printf("Waiting for incoming connections...\n");

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
		write(newPlayer->socket ,&sendClient , sizeof(message));
                break;
	}

	if(read_size == 0)
	{
		fprintf(stderr, "Client disconnected\n");
		fflush(stderr);
	}
	else if(read_size == -1)
	{
		perror("recv failed");
	}
        else
            amount_players++;

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


