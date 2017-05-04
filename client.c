#include "client.h"
struct message mSent; // Structure used to send messages to the server
struct message mRecv; // Structure used to receive messages from the server
char server_ip[20]; // The server's IP address (127.0.0.1 for testing)
int port; // The server's port

int main(int argc , char *argv[])
{
	struct sockaddr_in server_addr; // The server's socket address
	int socket; // The socket used to communicate with the server (file descriptor)
	if(argc!=3){
		fprintf(stderr, "Usage %s ip port\n", argv[0]); // Usage check
		exit(EXIT_FAILURE);
	}
	strcpy(server_ip, argv[1]);
	port = atoi(argv[2]);
	connect_to_server(&socket, &server_addr);
        if(receive_message( &mRecv, socket)==TRUE)
                printf("%s", mRecv.payload);
	signup(&socket);
	while(TRUE){
            sleep(1);//to prevent cpu overheat
            if(receive_message( &mRecv, socket)==TRUE)
                printf("%s", mRecv.payload);
        }
	shutdown_socket(socket);
	return 0;
}

void signup(int * client_socket){
	int inscriptionOK=FALSE;
	char messageS[MESSAGE_MAX_LENGTH];
	while(inscriptionOK==FALSE){
		printf("Enter your name : ");
		scanf("%s" , messageS);
		strcpy(mSent.payload, messageS);
		mSent.code=C_ADD_PLAYER;
                send_message( mSent, *client_socket);
		if(mRecv.code==C_OK)
			inscriptionOK=TRUE;
		printf("%s\n", mRecv.payload);
	}
}

void connect_to_server(int *client_socket , struct sockaddr_in *server_addr){

	//Create socket
	*client_socket = socket(AF_INET , SOCK_STREAM , 0);
	if (*client_socket == -1)
	{
		printf("Could not create socket");
	}
	printf("Socket created\n");

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
