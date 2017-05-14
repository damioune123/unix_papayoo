/**
*
*  AUTHORS : MANIET Alexandre (amaniet2015)(serie 2), MEUR Damien (dmeur15)(serie 2)
*  This file contains the source code of the server side of the program.
*
*/
#include "serveur.h"

player players[MAX_PLAYERS];
int port;
FILE *fpError;
int server_running, game_running, amount_players, server_socket;
int stock_addr_size = sizeof(struct sockaddr_in);
message mess;
struct timeval timeout = {0, 200000};//time to wait to recv essage before cancelling the operation (here 200 ms)

/* The deck's logical size */
int deck_logical_size = 0;
int papayoo; 
/* The deck */
card deck[DECK_PHYSICAL_SIZE];

int main(int argc , char *argv[])

{
        struct sigaction alarm, interrupt;
        int i, max_fd, select_res;
        fd_set fds;
	struct sockaddr_in server_addr, client_addr;
        int fdLock = open(SERVER_LOCK, O_RDWR);
        fct_ptr dispatcher[] = {add_player};//USE TO DIRECTLY CALL FUNCTION WITH A CODE SENT FROM CLIENT

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
        port=atoi(argv[1]);
        reset_players();
        amount_players=0;
        server_running=TRUE;
        game_running=FALSE;
	alarm.sa_handler = &alarm_handler;
	interrupt.sa_handler = &interrupt_handler;
        interrupt.sa_flags=0;
        sigemptyset(&interrupt.sa_mask);
        sigaddset(&interrupt.sa_mask, SIGALRM);//prevents alarm to cancel a shutdown operation
	sigaction(SIGALRM, &alarm, NULL);
	sigaction(SIGINT, &interrupt, NULL);
	sigaction(SIGTERM, &interrupt, NULL);
	sigaction(SIGQUIT, &interrupt, NULL);
        init_server(&server_socket, &server_addr, port, MAX_PLAYERS);
        create_segment();
        init_semaphores();
        while(server_running){
            usleep(100); //top prevent cpu overheat
            FD_ZERO(&fds);
            FD_SET(server_socket, &fds);
            max_fd = server_socket + 1;
            for (i = 0; i < MAX_PLAYERS; i++) {
                if (players[i].socket > 0) {
                    FD_SET(players[i].socket, &fds);
                }
                if (players[i].socket >= max_fd) {
                    max_fd = players[i].socket+1;
                }
            }
            if ((select_res = select(max_fd, &fds, NULL, NULL, &timeout)) < 0) {
                if (errno != EINTR) {
                    shutdown_server();
                    return EXIT_FAILURE;
                }
            }

            if (select_res > 0) {
                if (FD_ISSET(server_socket, &fds)) {
                    add_client(server_socket, &client_addr);
                }
                for (i = 0; i < amount_players; i++) {
                    if (FD_ISSET(players[i].socket, &fds)) {
                        if (receive_message(&mess, players[i].socket)) {
                            dispatcher[mess.code] (players[i].socket, mess);
                        }
                        else{
                            remove_player(players[i].socket);
                        }
                    }
                }
            }
            if(game_running){
            }
        }
        if(argc==3)
            fclose(fpError);
        if(close(fdLock)==-1){
            perror("Error closing lock file\n");
            return EXIT_FAILURE;
        }
        shutdown_socket(server_socket);
        shutdown_server();
	return EXIT_SUCCESS;
}
void add_client(int server_socket, struct sockaddr_in *cl_addr) {
	int new_cl_socket;
	int cl_addr_length = sizeof(struct sockaddr_in);
	if ((new_cl_socket = accept(server_socket, (struct sockaddr *)cl_addr, (socklen_t*) &cl_addr_length)) < 0) {
		perror("Connection error");
		exit(EXIT_FAILURE);
	} else {
		if (game_running || amount_players == MAX_PLAYERS ) {
                        mess.code=C_REFUSE;
                        strcpy(mess.payload,M_CONN_REFUSE);
			send_message(mess, new_cl_socket);

		} else {
                        printf("A client has connected\n");
                        players[amount_players++].socket = new_cl_socket;
                        if (amount_players == 1) 
                            alarm(COUNTDOWN);
                        mess.code=C_OK;
                        strcpy(mess.payload, M_GREET_CLIENT);
                        send_message(mess,new_cl_socket);
                }
        }
}


void add_player(int socket, message mesRecv) {
        int idx_player = find_player_id_by_socket(socket);
        if(idx_player ==-1){
            mess.code=C_SERVER_ERROR;
            strcpy(mess.payload, M_SERVER_ERROR);
            fprintf(stderr, "The server couldn't bind the socket with a player\n");
            send_message(mess, socket);
            return;
        }
        players[idx_player].is_registered=TRUE;
        strcpy(players[idx_player].name, mesRecv.payload);
        mess.code=C_OK;
        strcpy(mess.payload, M_SIGNUP_CLIENT_OK);
        send_message(mess, socket);
        printf("Player %s has joined the lobby\n", players[idx_player].name);
        printf("Current players :\n");
        for(int i =0; i < amount_players ; i++){
            if(players[i].is_registered)
                printf("Player number %i : %s is registered \n",i, players[i].name);
            else
                printf("Player number %i : is not registered yet\n", i);
        }
        if(amount_players >=2 && all_players_registered())
            start_game();
}
int find_player_id_by_socket(int socket){
    for(int j = 0; j < MAX_PLAYERS; j++){
        if(players[j].socket == socket)
                return j;
    }
    return -1;
}


void reset_players(){
        for(int  i=0; i < MAX_PLAYERS; i++){
            reset_player(&players[i]);
        }
        amount_players=0;
}
void reset_player(player *pl){
    pl->socket=0;
    pl->name[0]='\0';
    pl->is_registered=FALSE;
}

void shutdown_server() {
        
        sprintf(mess.payload,"The server has shut down\n");
        mess.code=C_SERVER_SHUT_DOWN;
        send_message_everybody(mess);
	printf("server shutting down ..\n");
	clear_lobby();
        kill_ipcs();
        server_running = FALSE;
}



void clear_lobby() {
    for(int i=0; i < amount_players; i++){
        shutdown_socket(players[i].socket);
    }
    reset_players();
    game_running=FALSE;
}
int all_players_registered(){
    for(int i=0; i < amount_players; i++){
        if(!players[i].is_registered)
            return FALSE;
    }
    return TRUE;
}


void remove_player( int socket) {
    char namePl[255];
    int idx_player = find_player_id_by_socket(socket);
    if(idx_player ==-1){
        fprintf(stderr, "The server couldn't bind the socket with a player\n");
        return;
    }
    shutdown_socket(players[idx_player].socket);
    players[idx_player].socket=0;
    if(players[idx_player].is_registered)
        strcpy(namePl, players[idx_player].name);
    else
        strcpy(namePl, "unregistered (Anonymous)");
    reset_player(&players[idx_player]);
    for(int j=idx_player;j< amount_players; j++ ){
        players[j]=players[j+1];
    }
    amount_players--;
    printf("Player %s has been successfully removed from the game \n", namePl);
    sprintf(mess.payload,"The player %s has left  the game\n", namePl);
    mess.code=C_INFO;
    send_message_everybody(mess);
    if(game_running){
        sprintf(mess.payload,"The game has been stopped due to a client disconnection\n", namePl);
        mess.code=C_GAME_CANCELLED;
        send_message_everybody(mess);
        clear_lobby();
    }
}


void alarm_handler(int signum) {
    if(game_running)
        return;
    mess.code=C_INFO;
    if (amount_players < 2) {
        sprintf(mess.payload,"The game hasn't started because  the  %i s countdown has expired. Amount of players in the lobby : %i\n. Countdown restarted\n", COUNTDOWN,  amount_players);
        alarm(COUNTDOWN);
        send_message_everybody(mess);
    }
    else if(all_players_registered()==FALSE){
        sprintf(mess.payload,"The game hasn't started because the required amount of players is full but some of them haven't registerd yet\n Countdown restarted\n");
        alarm(COUNTDOWN);
        send_message_everybody(mess);
    }
    else {
        start_game();
    }
}
void interrupt_handler(int signum) {
    switch(signum){
        case SIGINT:
            printf("SIGINT detected by the server\n");
            break;
        case SIGTERM:
            printf("SIGTERM detected by the server\n");
            break;
        case SIGQUIT:
            printf("SIGQUIT detected by the server\n");
            break;
        default:
            fprintf(stderr, "The signal number %i was detected but no routine was found to handle it ..\n", signum);
            break;
    }
    shutdown_socket(server_socket);
    shutdown_server();
}
void start_game() {
    printf("A game is starting with %i players.\n", amount_players);
    sprintf(mess.payload,"The Game starts now !\n Amount of players in the game : %i\n May the best win !\n", amount_players);
    send_message_everybody(mess);
    game_running = TRUE;
    init_shared_memory();
    start_round();
}


void start_round() {
    init_deck();
    shuffle_deck();
    show_cards((card*)deck,deck_logical_size);
    find_papayoo();
    deal_cards();
}

void deal_cards() {
    mess.code=C_INIT_DECK_RECEIVED;
    int amount_cards_player= DECK_PHYSICAL_SIZE/amount_players;
    int idx=0;
    for(int i=0; i < amount_players ; i++){
        for(int j = 0; j < amount_cards_player ;j ++){
            memcpy(&mess.deck[j],&deck[idx], sizeof(card));
            idx++;
        }
        mess.deck_logical_size = amount_cards_player;
        send_message(mess, players[i].socket);
    }
}

void send_message_everybody(message msg){
    for(int i=0; i < amount_players; i++){
        send_message(msg, players[i].socket);
    }
}

void init_shared_memory(){
    int i;
    for(i=0; i < amount_players ; i++){
        s_write_score(i,0);
        s_write_name(i, players[i].name);
    }
    //DEBUG
    /*
    char names[MAX_PLAYERS][BUFFER_SIZE];
    int scores[MAX_PLAYERS];
    s_read_names((char **)names);
    s_read_scores((int **) scores);
    for(i =0; i < amount_players; i++){
        printf("NOM %s // score %i \n", names[i], scores[i]);
    }*/
}

/* Initializes the deck with all 52 possible values in order */
void init_deck(){
        unsigned int i;
        char buffer[BUFFER_SIZE];
        for(i=1; i <=10 ; i++){
		card newCard; // creating new card instance;
                newCard.number=i;
                newCard.type =  SPADES_CONST;
                show_card(newCard, buffer);
                add_card(newCard);
        }
        for(i=1; i <=10 ; i++){
		card newCard; // creating new card instance;
                newCard.number=i;
                newCard.type = CLUBS_CONST;
                add_card(newCard);
        }
        for(i=1; i <=10 ; i++){
		card newCard; // creating new card instance;
                newCard.number=i;
                newCard.type= HEARTS_CONST;
                add_card(newCard);
        }
        for(i=1; i <=10 ; i++){
		card newCard; // creating new card instance;
                newCard.number=i;
                newCard.type= DIAMONDS_CONST;
                add_card(newCard);
        }
        for(i=1; i <=20 ; i++){
		card newCard; // creating new card instance;
                newCard.number=i;
                newCard.type= PAYOO_CONST;
                add_card(newCard);
        }

}

/* Shuffles all remaining cards in the deck  */
void shuffle_deck(){
	srand((unsigned int)time(NULL)); // getting seed for random number
        int count=0;	

	// running through all cards in deck and placing them in random spots in temp deck
	while(count <= SHUFFLE_CONST) { // as long as we've not placed all cards into the new array
                count++;
		int idx1 = rand() % deck_logical_size;
		int idx2 = rand() % deck_logical_size; 
                card temp_card;
                memcpy(&temp_card, &deck[idx1], sizeof(card));
                memcpy(&deck[idx1], &deck[idx2], sizeof(card));
                memcpy(&deck[idx2], &temp_card, sizeof(card));
	}
}

/* Retrieves the first (index 0) card in the deck, then shifts all cards back by 1 in the array. Returns the picked card if operation is successful.*/
card pick_card(){
	if(deck_logical_size <= 0){ // deck is empty
		fprintf(stderr , "Error during pick_card, deck empty.\n");
	}
	card firstCard = deck[0]; // fetching first card of deck
	for(int i = 0; i < deck_logical_size; i++){ // running through all cards in deck
		memcpy(&deck[i],&deck[i+1],sizeof(card)); // shifting cards back in the array
	}
	deck_logical_size--; // decrementing logical size
	return firstCard;
}

/* Adds one card to the logical end of the deck (bottom of the stack). Returns the added card if the operation was successful. */
card add_card(card newCard){
	if(deck_logical_size == DECK_PHYSICAL_SIZE){ // deck is full
		fprintf(stderr, "Error during add_card, deck full.\n");
	}
	memcpy(&deck[deck_logical_size++],&newCard, sizeof(card)); // adds card then increments logical size
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
void find_papayoo(){
        papayoo = rand() % 4; 
        mess.code=C_INFO;
        char buffer[BUFFER_SIZE];
        switch(papayoo){
            case SPADES_CONST:
                sprintf(buffer,"PAPAYOO is %s\n", SPADES);
                printf("%s\n", buffer);
                strcpy(mess.payload, buffer);
                send_message_everybody(mess);
                break;
            case HEARTS_CONST:
                sprintf(buffer,"PAPAYOO is %s\n", HEARTS);
                printf("%s\n", buffer);
                strcpy(mess.payload, buffer);
                send_message_everybody(mess);
                break;
            case CLUBS_CONST:
                sprintf(buffer,"PAPAYOO is %s\n", CLUBS);
                printf("%s\n", buffer);
                strcpy(mess.payload, buffer);
                send_message_everybody(mess);
                break;
            case DIAMONDS_CONST:
                sprintf(buffer,"PAPAYOO is %s\n", DIAMONDS);
                printf("%s\n", buffer);
                strcpy(mess.payload, buffer);
                send_message_everybody(mess);
                break;
            default:
                fprintf(stderr,"Error choosing papayoo\n");
                exit(EXIT_FAILURE);
         }

}
