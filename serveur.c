/**
 *
 *  AUTHORS : MANIET Alexandre (amaniet2015)(serie 2), MEUR Damien (dmeur15)(serie 2)
 *  This file contains the source code of the server side of the program.
 *
 */
#include "serveur.h"
static player players[MAX_PLAYERS];
static int server_running, game_running, amount_players, server_socket, port, papayoo;
static int stock_addr_size = sizeof(struct sockaddr_in);
static message mess;
static struct timeval timeout = {0, 200000};//time to wait to recv essage before cancelling the operation (here 200 ms)
static card deck[DECK_PHYSICAL_SIZE];
static int deck_logical_size = 0;
static boolean one_ecart_received = FALSE;
static int amount_ecart_received=0;
static ecart ecarts_received[MAX_PLAYERS];
static int current_round=0;;
static int current_player_turn=0;
static boolean one_card_played_received = FALSE;
static int amount_cards_played_this_turn=0;
static int amount_turn=0;
static boolean updating_score=FALSE;
static int amount_scores_updated=0;
int main(int argc , char *argv[]){
    FILE *fpError;
    struct sigaction alarm, interrupt;
    int i, max_fd, select_res;
    fd_set fds;
    struct sockaddr_in server_addr, client_addr;
    int fdLock = open(SERVER_LOCK, O_RDWR);
    fct_ptr dispatcher[] = {add_player, receive_ecart_from_player, receive_played_card, update_score};//USE TO DIRECTLY CALL FUNCTION WITH A CODE SENT FROM CLIENT
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
        if(one_ecart_received && amount_ecart_received == amount_players){
            send_ecart_back();
            ask_for_card(current_player_turn);
        }
        if(updating_score && amount_scores_updated == amount_players){
            start_round;
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
/**
 *
 * This function allows the server to connect to a client. After a client has connected, his socket file descriptor is kept in the players array. One the client has been added, is not registered yet. If the game is running or the maximum amout of players is reached, the client will be refused.
 *
 * @param int server_socket : the socket's file descriptor of the server
 *
 * @param struct sockaddr_in : an empty  sockaddr_in structure 
 *
 *
 */
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
/**
 *
 * This function is called whenever a connected lient want to register for a game using his name.
 *
 * @param int socket : the socket's file descriptor of the connected client
 *
 * @param message mesRecv : the message structure (see message.h) sent by the client containing his name.
 *
 */
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
}
/**
 *
 * This function find the index of the player in the players array using his socket's file descriptor
 *
 * @param int socket : the client socket's file descriptor
 *
 * @return : the player index or -1 if not found
 *
 */
int find_player_id_by_socket(int socket){
    for(int j = 0; j < MAX_PLAYERS; j++){
        if(players[j].socket == socket)
            return j;
    }
    return -1;
}
/**
 *
 * This function is used to receive a deck of 5 cards from a player (an ecart).
 *
 * @param int socket : the socket's file descriptor of the player sending his ecart
 * @param message msg : the message structure containing the 5 cards
 *
 *
 */
void receive_ecart_from_player(int socket, message msg){
    int player_id = find_player_id_by_socket(socket);
    one_ecart_received = TRUE;
    amount_ecart_received++;
    for(int i=0; i < 5; i++){
        memcpy(&ecarts_received[player_id].cards[i], &msg.deck[i], sizeof(card));
    }
}

/**
 *
 * This function is used to send back the differents ecarts to the players, after they all have been sent.
 *
 *
 */
void send_ecart_back(){
    mess.code = C_ALL_ECART_DECK_RECEIVED;
    mess.deck_logical_size = 5;
    for(int i =0; i < amount_players ; i++){
        for(int j=0; j < 5 ; j++){
            memcpy(&mess.deck[j], &ecarts_received[i].cards[j], sizeof(card));
        }
        send_message(mess, players[(i+1)%amount_players].socket);
    }
    one_ecart_received=FALSE;
    amount_ecart_received=0;
}
/**
 *
 * This function clean up all the players data in the players array
 *
 *
 */
void reset_players(){
    for(int  i=0; i < MAX_PLAYERS; i++){
        reset_player(&players[i]);
    }
    amount_players=0;
}
/**
 *
 * This function clean up a single player data.
 *
 *
 */
void reset_player(player *pl){
    pl->socket=0;
    pl->name[0]='\0';
    pl->is_registered=FALSE;
}
/**
 *
 * This function clean up server data before shutting down (and notice the connected clients)
 *
 *
 */
void shutdown_server() {
    sprintf(mess.payload,"The server has shut down\n");
    mess.code=C_SERVER_SHUT_DOWN;
    send_message_everybody(mess);
    printf("server shutting down ..\n");
    clear_lobby();
    kill_ipcs();
    server_running = FALSE;
}
/**
 *
 * This function clean the client's sockets, clean up the players' data and stop the running game.
 *
 *
 */
void clear_lobby() {
    for(int i=0; i < amount_players; i++){
        shutdown_socket(players[i].socket);
    }
    reset_players();
    game_running=FALSE;
}
/**
 * This function aims to say if all the connected clients have registered (using their names)
 *
 * @return int : TRUE if all clients have registered, FALSE else
 *
 *
 */
boolean  all_players_registered(){
    for(int i=0; i < amount_players; i++){
        if(!players[i].is_registered)
            return FALSE;
    }
    return TRUE;
}
/**
 *
 * This function remove a single player from the game . It handles whether a game is running (stops it and notice other players) or not.
 *
 * @param int socket : the socket's file descriptor of the player to remove
 *
 *
 */

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
/**
 *
 * This function is called whenever a SIGALRM signal is caught, it aims to prevent the server to systematically refusing more than 2 players
 *
 * @param int signum: the caught  signal number (always SIGALRM)
 *
 *
 */
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
/**
 *
 * This function is called whenever a SIGINT, SIGQUIT OR SIGTERM signal is caught. It aims to shutdown the server cleanly.
 *
 * @param int signum : the caught signal number
 *
 *
 *
 */
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
/**
 * This function aims launch a new game when all the clients have registered and the minimum amount of players is reached. It notices all players , initialize shared memory and start the first round.
 *
 *
 */
void start_game() {
    printf("A game is starting with %i players.\n", amount_players);
    sprintf(mess.payload,"The Game starts now !\n Amount of players in the game : %i\n May the best win !\n", amount_players);
    send_message_everybody(mess);
    game_running = TRUE;
    init_shared_memory();
    start_round();
}
/**
 * 
 * This function launches a new round. It initalizes deck, shuffles it, finds papayoo, deals cards.
 *
 */
void start_round() {
    current_round++;
    init_deck();
    shuffle_deck();
    find_papayoo();
    send_basic_info();
    deal_cards();
}
/**
 *
 * This function is called after the whole deck is shuffled and split it between all players.
 *
 */
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
/**
 *
 * This function is used to send a message to every connected clients.
 *
 * @param message msg : the message structure (see message.h) to send
 */
void send_message_everybody(message msg){
    for(int i=0; i < amount_players; i++){
        send_message(msg, players[i].socket);
    }
}
/**
 *
 * This function reset scores and writes every players' name contained in the players array in shared memory
 *
 *
 *
 */
void init_shared_memory(){
    int i;
    for(i=0; i < amount_players ; i++){
        s_write_score(i,0);
        s_write_name(i, players[i].name);
    }
}
/**
 *
 * This function initializes the deck (still ordered)
 *
 */
void init_deck(){
    unsigned int i;
    char buffer[BUFFER_SIZE];
    for(i=1; i <=10 ; i++){
        card newCard; // creating new card instance;
        newCard.number=i;
        newCard.type =  SPADES_CONST;
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
/**
 *
 * This function shuffle the deck, the SHUFFLE_CONST can be incremented to raise entropy
 *
 *
 */
void shuffle_deck(){
    srand((unsigned int)time(NULL)); // getting seed for random number
    int count=0;	

    while(count <= SHUFFLE_CONST) { 
        count++;
        int idx1 = rand() % deck_logical_size;
        int idx2 = rand() % deck_logical_size; 
        card temp_card;
        memcpy(&temp_card, &deck[idx1], sizeof(card));
        memcpy(&deck[idx1], &deck[idx2], sizeof(card));
        memcpy(&deck[idx2], &temp_card, sizeof(card));
    }
}
/**
 *
 * This function is used to add a card at the bottom of the deck (only used during initialization)
 *
 * @param card newCard : the card to add to the deck
 *
 */
card add_card(card newCard){
    if(deck_logical_size == DECK_PHYSICAL_SIZE){ // deck is full
        fprintf(stderr, "Error during add_card, deck full.\n");
    }
    memcpy(&deck[deck_logical_size++],&newCard, sizeof(card)); // adds card then increments logical size
}
/**
 *
 * This function show every cards of the deck, only used for debugging in the server side.
 *
 * @param cards* deck : the deck to show
 * 
 * @param int logical_size : logical size of the deck to show
 *
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
 * This functions shows a single card (only used for debugging in the server side)
 * @param card cardToShow : the card to show
 *
 * @param char *display : the buffer to put the information of the card.
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
/**
 *
 * This function finds randomly the type of papayoo
 *
 */
void find_papayoo(){
    papayoo = rand() % 4; 
}
void send_basic_info(){
    mess.code=C_BASIC_INFO;
    mess.info.amount_players=amount_players;
    mess.info.papayoo=papayoo;
    mess.info.current_round=current_round;
    for(int i=0; i < amount_players ; i++){
        mess.info.player_index=i;
        send_message(mess, players[i].socket);
    }
}
/**
 * 
 * This function send a message to a player to ask him to send a card.
 *
 * @param int player_idx : the player's index to ask ask for card
 *
 */
void ask_for_card(int player_idx){
    mess.code = C_ASK_FOR_CARD;
    send_message(mess,players[player_idx].socket);
}

/**
 *
 * This function is called when a player has played a card.
 *
 * @param int socket : socket's file descriptor of the player
 *
 * @param message msg : the message containing the played card
 *
 */
void receive_played_card(int socket, message msg){
    s_write_card(msg.deck[0]);
    one_card_played_received = TRUE;
    amount_cards_played_this_turn++;
    int next_player = (msg.deck[0].last_played+1)%amount_players;
    if(one_card_played_received && amount_cards_played_this_turn == amount_players){
        end_turn();
    }
    else{
        mess.code=C_SHOW_PLI;
        send_message_everybody(mess);
        ask_for_card(next_player);
    }
}
/**
 *
 *  This functions ends a turn. It  finds out the looser of the pli and sends him the current pli.
 *
 *
 */
void end_turn(){
    amount_turn++;
    card pli[MAX_PLAYERS];
    s_read_cards((card**) &pli);
    int turn_type = pli[0].type;
    int max_value = pli[0].number;
    int looser = pli[0].last_played;
    for(int i=1;i< amount_players;i++){
        if(pli[i].type ==turn_type && pli[i].number > max_value){
            looser =pli[i].last_played;
            max_value = pli[i].number;
        }
    }
    current_player_turn = looser;
    one_card_played_received =FALSE;
    amount_cards_played_this_turn =0;
    send_pli(looser);
    s_reset_card_size();
    int total_turn = DECK_PHYSICAL_SIZE / amount_players;
    if(amount_turn == total_turn)
         end_round();
    else
        ask_for_card(looser);
}
/**
 *
 * This function sends player the current pli and he will put it  in his pli deck
 *
 * @param player_index : the index of the player who lost the turn
 *
 */
void send_pli(int player_index){
    card pli[MAX_PLAYERS];
    s_read_cards((card **) &pli);
    mess.code = C_ADD_PLI;
    int pli_size = s_read_cards_size();
    for(int i =0; i < pli_size ; i++){
        memcpy(&mess.deck[i], &pli[i], sizeof(card));
    }
    mess.deck_logical_size=pli_size;
    send_message(mess, players[player_index].socket);

}
/**
 * 
 * This functions ends the round. It asks players to send their score and reset round information
 *
 */
void end_round(){
    current_player_turn=0;    
    //TODO reset information ?

}
/**
 *
 * This function asks all the players to send their score.
 *
 */
void ask_for_score(){
    mess.code = C_ADD_SCORE;
    amount_scores_updated =0;
    updating_score=TRUE;
    send_message_everybody(mess);
}
/**
 * 
 * This function update the score of a player in shared memory
 *
 */
void update_score(int socket, message msg){
    int scores_temp[MAX_PLAYERS];
    int player_idx = find_player_id_by_socket(socket);
    int current_score;
    s_read_scores((int **) &scores_temp);
    current_score = scores_temp[player_idx]+msg.score;
    s_write_score(player_idx, current_score);
    printf("DEBUG score of player %s is now %i\n", players[player_idx].name, current_score);

}
