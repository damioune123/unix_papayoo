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
int deck_logical_size;
card deck[DECK_PHYSICAL_SIZE];
int main(int argc , char *argv[])
{

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
        locate_segment();
        locate_semaphores();
        try_to_connect(&socketC, &server_addr);
        signup(&socketC);
        while(TRUE){
            usleep(50);//to prevent cpu overheat
            if(receive_message( &mRecv, socketC)==TRUE){
                switch (mRecv.code){
                    case C_INFO:
                        printf("%s", mRecv.payload);
                        break;
                    case C_GAME_CANCELLED:
                        printf("%s", mRecv.payload);
                        shutdown_socket(socketC);
                        socketC=0;
                        try_to_connect(&socketC, &server_addr);
                        signup(&socketC);
                        break;
                    case C_SERVER_SHUT_DOWN:
                        printf("%s", mRecv.payload);
                        shutdown_socket(socketC);
                        return EXIT_FAILURE;
                    case C_INIT_DECK_RECEIVED:
                        init_deck(mRecv.deck, mRecv.deck_logical_size);
                        break;
                    default:
                        continue;
                }
            }

        }
	return EXIT_SUCCESS;
}
void init_deck(card * cards_sent, int cards_sent_size){
    deck_logical_size = cards_sent_size;
    for(int i=0; i < deck_logical_size ;i++){
        memcpy(&deck[i], &cards_sent[i], sizeof(card));
    }
    choose_ecart();
}
void choose_ecart(){
    printf("Here are your cards, please choose 5 for ecart (type in the cards number with space betwen them)\n");
    show_cards(deck, deck_logical_size);

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
void show_cards(card* deck, int logical_size){
    char display[BUFFER_SIZE];
    for(int i = 0; i < logical_size; i++){
        show_card(deck[i], display);
        printf("CARD %i : %s\n",i+1, display);
    }
}

void show_card(card cardToShow, char * display){
    switch(cardToShow.type){
        case SPADES_CONST:
            sprintf(display, "%i of %s", cardToShow.number, SPADES);
            break;
        case HEARTS_CONST:
            sprintf(display, "%i of %s", cardToShow.number, HEARTS);
            break;
        case CLUBS_CONST:
            sprintf(display, "%i of %s", cardToShow.number, CLUBS);
            break;
        case DIAMONDS_CONST:
            sprintf(display, "%i of %s", cardToShow.number, DIAMONDS);
            break;
        case PAYOO_CONST:
            sprintf(display, "%i of %s", cardToShow.number,PAYOOS );
            break;
        default:
            fprintf(stderr, "Wrong card const\n");
            exit(EXIT_FAILURE);
    }
}
