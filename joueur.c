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
static card deck[DECK_PHYSICAL_SIZE/2];// the player's deck
static boolean waiting_for_ecart = FALSE;//used after sending ecart and waiting for server to giving own ecart to player
static char players_names[MAX_PLAYERS][BUFFER_SIZE];//used to read names from shared memory
static int scores[MAX_PLAYERS];//used to read scores from shared memory
static basic_info info; //basic information : player index, amount_players, current round, papayoo
static card plis[DECK_PHYSICAL_SIZE];
int pli_logical_size=0;
int main(int argc , char *argv[]){
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
                case C_ALL_ECART_DECK_RECEIVED:
                    add_new_ecart(mRecv.deck, mRecv.deck_logical_size);
                    break;
                case C_BASIC_INFO:
                    memcpy(&info, &mRecv.info,sizeof(basic_info));
                    show_info();
                    break;
                case C_ASK_FOR_CARD:
                    play_card();
                    break;
                case C_SHOW_PLI:
                    show_pli();
                    break;
                case C_ADD_PLI:
                    add_pli(mRecv.deck, mRecv.deck_logical_size);
                    break;
                default:
                    continue;
            }
        }
        if(waiting_for_ecart){
            printf("ALL players haven't sent their ecart yet. Please Wait ...\n Next check in %i seconds\n", TIME_TRY_CONNECT);
            sleep(TIME_TRY_CONNECT);
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
    send_ecart(socketC);
}
/**
 *
 * This function displays all the basic information of the players (reading shared memory) (scores/names/current rount/papayoo)
 *
 *
 */
void show_info(){
    s_read_names((char**)players_names);
    s_read_scores((int **) scores);
    printf("-------------------INFO REVIEW--------------------------\n");
    printf("Current scores :\n");
    for(int i=0; i < info.amount_players ; i++){
        if(i==info.player_index)
            printf("YOU : Name : %s | Score : %i\n", players_names[i], scores[i]);
        else
            printf("Name : %s | Score : %i\n", players_names[i], scores[i]);
    }
    printf("Current round : %i\n", info.current_round);
    switch(info.papayoo){
        case SPADES_CONST:
            printf("Papayoo is %s\n", SPADES);
            break;
        case HEARTS_CONST:
            printf("Papayoo is %s\n", HEARTS);
            break;
        case DIAMONDS_CONST:
            printf("Papayoo is %s\n", DIAMONDS);
            break;
        case CLUBS_CONST:
            printf("Papayoo is %s\n", CLUBS);
            break;
        default:
            fprintf(stderr," Wrong papayoo const\n");
            exit(EXIT_FAILURE);
    }
    printf("-------------------------------------------------------\n");
}
/**
 *
 * This function is used to get the ecart given by another player to the deck.
 *
 * @param card * cards_sent : the ecart sent
 *
 * @param int cards_sent_size : the size of the ecart (normally 5)
 *
 */
void add_new_ecart(card * cards_sent, int cards_sent_size){
    printf("Here are the cards sent by player %s \n", players_names[(info.player_index+1)%info.amount_players]);
    show_cards(cards_sent, cards_sent_size, 0);
    for(int i=0; i < cards_sent_size; i++){
        memcpy(&deck[deck_logical_size++], &cards_sent[i], sizeof(card));
    }
printf("Here is your complete deck for the round : \n");
    show_cards(deck, deck_logical_size, 0);
    show_info();
    waiting_for_ecart = FALSE;
}
/**
 *
 * This function is used to allow the player to choose 5 cards to give to another player at the beginning of a round. The cards will be chosen and then sent to the server that will give them to another player.
 * 
 */
void send_ecart(int client_socket){
    int *array_ints;
    char buffer[BUFFER_SIZE];
    if( (array_ints = (int *) malloc(sizeof(int)*5)) == NULL){
        fprintf(stderr, "Erreur malloc\n");
        exit(EXIT_FAILURE);
    }
    show_cards(deck, deck_logical_size, 0);
    printf("Here are your cards, please choose 5 for ecart (type in the cards number with space betwen them)\n");
    int ecart_ok=FALSE;
    while(TRUE){
        int ret;
        if( (ret=read(0, buffer, BUFFER_SIZE))==-1){
            fprintf(stderr, "Erreur read stdin\n");
            exit(EXIT_FAILURE);
        }
        else{
            buffer[ret-1]='\0';
            if(convert_input_to_integer_array(buffer, &array_ints))
                break;
            else
                printf("Please choose 5 card number from 1 to %i\n",deck_logical_size);
        }
    }
    //copying chosen carts in the array of cards to send to server
    int i;
    for(i=0; i< 5;i++){
        memcpy(&mSent.deck[i], &deck[array_ints[i]], sizeof(card));
    }
    mSent.deck_logical_size=5;
    mSent.code=C_ECART_DECK_SENT;
    printf("You've chosen the followin cards\n Sending to server ...\n");
    show_cards(mSent.deck, mSent.deck_logical_size, 0);
    remove_ecart(array_ints);
    send_message(mSent, client_socket);
    waiting_for_ecart = TRUE;
}
/**
 *
 * This function removes the ecart from the deck.
 *
 * @param int *indexes : the indexes of the cards to remove
 *
 */
void remove_ecart(int *indexes){
    int i, j;
    int idx=0;
    card newDeck[deck_logical_size];
    for(i=0; i < deck_logical_size ; i++){
        boolean OK=TRUE;
        for(j=0; j < 5 ; j++){
            if(indexes[j]==i)
                OK=FALSE;
        }
        if(OK)
            memcpy(&newDeck[idx++], &deck[i], sizeof(card));
    }
    for(i=0; i < deck_logical_size; i++){
        memcpy(&deck[i], &newDeck[i], sizeof(card));
    }
    deck_logical_size-=5;
}
/**
 *
 * This function converts a char * into an array of 5 integers.
 *
 * @param char *input : the char * input
 *
 * @param int ** array : the array of integers to fill in
 *
 * @return  TRUE: if the char * contains 5 integers betwen 1 and the deck size, FALSE else
 *
 */
boolean convert_input_to_integer_array(char * input, int ** array){
    char *numberS;
    int number;
    int number_typed_in=0;
    char seps[] = ",\t\n ";
    numberS = strtok(input, seps);
    number = atoi(numberS);
    if(number <1 || number > deck_logical_size)
        return FALSE;
    (*array)[number_typed_in++] = number-1;
    while((numberS = strtok(NULL, seps))!=NULL){
        number = atoi(numberS);
        if(number <1 || number > deck_logical_size)
            return FALSE;
        (*array)[number_typed_in++] = number-1;
        if(number_typed_in >5){
            return FALSE;
        }
    }
    //seek doublons
    for(int i=0; i <5 ; i++){
        for(int j=0; j<5;j++){
            if(i!=j){
                if((*array)[i]==(*array)[j])
                    return FALSE;
            }
        }
    }
    return TRUE;
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
 *
 * @param int option : 0 = don't display player who last played the card, 1 =do
 */
void show_cards(card* deck, int logical_size, int option){
    char display[BUFFER_SIZE];
    for(int i = 0; i < logical_size; i++){
        show_card(deck[i], display, option);
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
 * @param int option : 0 = don't display player who last played the card, 1 =do
 *
 */
void show_card(card cardToShow, char * display, int option){
    if(option ==0){
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
    else{
        switch(cardToShow.type){
            case SPADES_CONST:
                sprintf(display, "%i of %s, played by %s", cardToShow.number, SPADES, players_names[cardToShow.last_played] );
                break;
            case HEARTS_CONST:
                sprintf(display, "%i of %s, played by %s", cardToShow.number, HEARTS, players_names[cardToShow.last_played] );
                break;
            case CLUBS_CONST:
                sprintf(display, "%i of %s, played by %s", cardToShow.number, CLUBS, players_names[cardToShow.last_played] );
                break;
            case DIAMONDS_CONST:
                sprintf(display, "%i of %s, played by %s", cardToShow.number, DIAMONDS,  players_names[cardToShow.last_played] );
                break;
            case PAYOO_CONST:
                sprintf(display, "%i of %s, played by %s", cardToShow.number,PAYOOS, players_names[cardToShow.last_played] );
                break;
            default:
                fprintf(stderr, "Wrong card const\n");
                exit(EXIT_FAILURE);
        }
    }
}
/**
 *
 * This function asks the player to play a card and sends it to the server.
 * The card is then remove from the player's deck
 *
 */
void play_card(){
    show_cards(deck, deck_logical_size, 0);
    printf("Here above is your current deck, please choose a card to play\n");
    int ret;
    int card_idx;
    char buffer[BUFFER_SIZE];
    while(TRUE){
        if( (ret = read(0, buffer, BUFFER_SIZE)) ==-1){
            fprintf(stderr,"Error read stdin\n");
            exit(EXIT_FAILURE);
        }
        else{
            buffer[ret-1]='\0';
            card_idx = atoi(buffer);
            if(card_idx>=1 && card_idx <=deck_logical_size)
                break;
            printf("Please choose a card between 1 and %i\n", deck_logical_size);
        }
    }
    show_card(deck[--card_idx], buffer, 0);
    printf("You choose to play :\n%s\n", buffer);
    deck[card_idx].last_played=info.player_index;
    memcpy(&mSent.deck[0], &deck[card_idx], sizeof(card));
    remove_card(card_idx);
    mSent.code=C_PLAY_CARD;
    send_message(mSent, socketC);

}
/**
 *
 * This function shows the current pli.
 *
 *
 */
void show_pli(){
    card pli[MAX_PLAYERS];
    s_read_cards((card **) &pli);
    printf("------------------ Current Pli --------------------\n");
    show_cards(pli, s_read_cards_size(), 1);
    printf("---------------------------------------------------\n");
    fflush(stdin);

}
/***
 *
 * This function removes a card from the player's deck.
 *
 * @param int card_idx : the card to remove.
 *
 *
 */
void remove_card(int card_idx){
    for(int i=card_idx; i< deck_logical_size -1 ; i++){
        memcpy(&deck[i], &deck[i+1], sizeof(card));
    }
    deck_logical_size--;

}
void add_pli(card * pli_sent, int pli_sent_size){
    printf("You lost the turn, following cards will be added to you plis' deck\n");
    show_cards(pli_sent, pli_sent_size, 0);
    for(int i= pli_logical_size; i < pli_logical_size + pli_sent_size; i++){
        memcpy(&plis[i], &pli_sent[i%pli_sent_size], sizeof(card));
    }
    pli_logical_size += pli_sent_size;
    //DEBUG
    printf("DEBUG :YOUR CURRENT PLIS DECK : %i %i\n", pli_logical_size, pli_sent_size);
    show_cards(plis, pli_logical_size, 0);
    printf("END DEBUG\n");

}
