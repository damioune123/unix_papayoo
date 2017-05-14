/**
*
*  AUTHORS : MANIET Alexandre (amaniet2015)(serie 2) , MEUR Damien (dmeur15)(serie 2)
*  This file contains the source code needed for the player side program.
*
*/

#include "joueur.h"
static struct message mSent; // Structure used to send messages to the server
static struct message mRecv; // Structure used to receive messages from the server
static char server_ip[20]; // The server's IP address (127.0.0.1 for testing)
static int port; // The server's port
static int socketC; // The socket used to communicate with the server (file descriptor)
static int deck_logical_size;// the logical size of the player's deck
static card deck[DECK_PHYSICAL_SIZE];// the player's deck
int main(int argc , char *argv[])
{

        struct sigaction interrupt;
        interrupt.sa_handler = &interrupt_handler;
        sigaction(SIGTERM, &interrupt, NULL);
        sigaction(SIGQUIT, &interrupt, NULL);
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
/**
 * This function will fill up the deck and deck_logical_size static vars with the ones sent by the server after it finishes dealing cards.
 * @param card *cards_sent : an array of cards sent by the server
 * @param int cards_sent_size : the logical size of the deck sent by the server
 *
 */
void init_deck(card * cards_sent, int cards_sent_size){
    deck_logical_size = cards_sent_size;
    for(int i=0; i < deck_logical_size ;i++){
        memcpy(&deck[i], &cards_sent[i], sizeof(card));
    }
    choose_ecart();
}
/**
 *
 * This function is used to allow the player to choose 5 cards to give to another player at the beginning of a round. The cards will be chosen and then sent to the server that will give them to another player.
 * 
 */
void choose_ecart(){
    printf("Here are your cards, please choose 5 for ecart (type in the cards number with space betwen them)\n");
    show_cards(deck, deck_logical_size);

}
/**
 *
 * This function will allow the current connected client to signup with his name to the server. Untill it hasn't done this operation, the player is considered as unregistered and the lobby will wait for him to start a game.
 * This function wait for the server to give him a response that says everything went ok.
 *
 * @param int * client_socket : the socket's file descriptor of the current unregistered but connected client
 *
 *
 */
void signup(int * client_socket){
	int inscriptionOK=FALSE;
	char messageS[MESSAGE_MAX_LENGTH];
	while(TRUE){
		printf("Enter your name : ");
		scanf("%s" , messageS);
		strcpy(mSent.payload, messageS);
		mSent.code=C_ADD_PLAYER;
                send_message( mSent, *client_socket);
		if(mRecv.code==C_OK)
                    break;
	}
}
/**
 *
 *  This function will connect the client with server.
 *
 *  @param int * socket : the empty socket file descriptor of the server (here it is a static var, but passed as parameter to make the code reusable)
 *
 *  @param struct sockadd_in : the empty structure sockaddr_in to fill in with the server_ip, port and transport protocol used (here TCP)
 *
 *
 */
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

/**
 * This function is called whenever a SIGINT, SIGTERM or SIGQUIT signal is sent by the user. It aims to make a clean exit (closing the client's socket before leaving.
 *
 * @param int signum : the signal number
 *
 */

void interrupt_handler(int signum){
    shutdown_socket(socketC);
    exit(EXIT_SUCCESS);
}
/**
 *
 * This funciton is used to show in the console the entire player's deck.
 *
 * @param card * deck : The array of cards (the deck) of the player (here it is a static var but passed as parameter to make the code reusable)
 *  
 * @param int logical_size : the logical size of the deck (here it is a static var but passed as parameter to make the code reusable)
 */
void show_cards(card* deck, int logical_size){
    char display[BUFFER_SIZE];
    for(int i = 0; i < logical_size; i++){
        show_card(deck[i], display);
        printf("CARD %i : %s\n",i+1, display);
    }
}
/**
 * 
 * This function is used to display a single card (the number of the card and its type).
 * 
 * @param card cardToShow : the card to show
 *
 * @param char * display : this is the buffer that's is going to be filled up with the card information
 *
 */
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
