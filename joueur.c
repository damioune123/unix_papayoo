/**
*
*  AUTHORS : MANIET Alexandre (amaniet2015)(serie 2) , MEUR Damien (dmeur15)(serie 2)
*  This file contains the source code needed for the player side program.
*
*/

#include "joueur.h"
struct message mSent; // Structure used to send messages to the server
struct message mRecv; // Structure used to receive messages from the server
char server_ip[20]; // The server's IP address (127.0.0.1 for testing)
int port; // The server's port
int socketC; // The socket used to communicate with the server (file descriptor)

int main(int argc , char *argv[])
{
        locate_segment();
        locate_semaphores();
        //test shared memory
        int test[4];
        s_read_scores((int**)&test);
        printf("%i %i %i %i \n", test[0], test[1], test[2], test[3]);

        struct sigaction interrupt;
        interrupt.sa_handler = &interrupt_handler;
        sigaction(SIGTERM, &interrupt, NULL);
        sigaction(SIGKILL, &interrupt, NULL);
        sigaction(SIGINT, &interrupt, NULL);
	struct sockaddr_in server_addr; // The server's socket address
	if(argc!=3){
		fprintf(stderr, "Usage %s ip port\n", argv[0]); // Usage check
		return EXIT_FAILURE;
	}
	strcpy(server_ip, argv[1]);
	port = atoi(argv[2]);
        try_to_connect(&socketC, &server_addr);
        signup(&socketC);
        while(TRUE){
            usleep(50);//to prevent cpu overheat
            if(receive_message( &mRecv, socketC)==TRUE){
                printf("%s", mRecv.payload);
                switch (mRecv.code){
                    case C_GAME_CANCELLED:
                        shutdown_socket(socketC);
                        socketC=0;
                        try_to_connect(&socketC, &server_addr);
                        signup(&socketC);
                        break;
                    case C_SERVER_SHUT_DOWN:
                        shutdown_socket(socketC);
                        return EXIT_FAILURE;
                    default:
                        continue;
                }
            }

        }
	return EXIT_SUCCESS;
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

void try_to_connect(int *socket, struct sockaddr_in *server_addr){
    while(TRUE){
        connect_to_server(socket, server_addr, server_ip, port);
        if(receive_message( &mRecv, *socket)){
            printf("%s", mRecv.payload);
            if(mRecv.code ==C_OK)
                break;
            printf("Another try in %i seconds\n", TIME_TRY_CONNECT);
        }
        sleep(TIME_TRY_CONNECT);
    }
}

void interrupt_handler(int signum){
    shutdown_socket(socketC);
    exit(EXIT_SUCCESS);
}

