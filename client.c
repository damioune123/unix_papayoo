#include "client.h"
struct message mSent; // Structure used to send messages to the server
struct message mRecv; // Structure used to receive messages from the server
int port; // The server's port
char server_ip[20]; // The server's IP address (127.0.0.1 for testing)
int socket; // The socket used to communicate with the server (file descriptor)
struct sockaddr_in server_addr; // The server's socket address
int main(int argc , char *argv[])
{
    if(argc!=3){
        fprintf(stderr, "Usage %s ip port\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    strcpy(server_ip, argv[1]);
    port = atoi(argv[2]);
    connect(&socket, &server_addr);
    signup();
    while(TRUE){
        printf("tjrs connecte\n");
    }
    close(socket);
    return 0;
}

void signup(){
	int inscriptionOK=FALSE;
	while(inscriptionOK==FALSE){
		printf("Enter your name : ");
		scanf("%s" , messageS);
		strcpy(mSent.payload, messageS);
		mSent.code=C_DEFAULT;
		//Send some data
		if( send(*client_socket , &mSent , sizeof(struct message) , 0) < 0)
		{
			printf("Send failed\n");
			continue;
		}

		//Receive a reply from the server
		if( recv(*client_socket , &mRecv , sizeof(struct message) , 0) < 0)
		{
			printf("recv failed\n");
			continue;
		}
		if(mRecv.code==C_OK)
			inscriptionOK=TRUE;
		printf("%s\n", mRecv.payload);
	}
}

void connect(int *client_socket , struct sockaddr_in *server_addr){
	char messageS[MESSAGE_MAX_LENGTH];

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

