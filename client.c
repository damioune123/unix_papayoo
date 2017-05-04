#include "client.h"
struct message mSent; // Structure used to send messages to the server
struct message mRecv; // Structure used to receive messages from the server
struct sigaction act; // The sigaction structure used to handle various signals
char server_ip[20]; // The server's IP address (127.0.0.1 for testing)
int port; // The server's port

// Sigaction handler
void hdl (int sig, siginfo_t *siginfo, void *context){ 
	printf("entered sigaction handler");
	//TODO add message with error code to server and exit with a specific code
}



int main(int argc , char *argv[])
{
	struct sockaddr_in server_addr; // The server's socket address
	int socket; // The socket used to communicate with the server (file descriptor)
	if(argc!=3){
		printf(stderr, "Usage %s ip port\n", argv[0]); // Usage check
		exit(EXIT_FAILURE);
	}
	memset (&act, '\0', sizeof(act)); // preparing sigaction
	// Use the sa_sigaction field because the handles has two additional parameters
	act.sa_sigaction = &hdl;
	// The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler.
	act.sa_flags = SA_SIGINFO;
	// catching SIGTERM
	if (sigaction(SIGTERM, &act, NULL) < 0) { 
		perror ("sigaction");
		return 1;
	}

	strcpy(server_ip, argv[1]);
	port = atoi(argv[2]);
	connect_to_server(&socket, &server_addr);
	signup(&socket);
	while(TRUE){}
	close(socket);
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

