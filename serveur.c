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
int main(int argc , char *argv[])
{
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
        struct sigaction alarm, interrupt;
        memset(&alarm, 0, sizeof(alarm));
	memset(&interrupt, 0, sizeof(interrupt));
	alarm.sa_handler = &alarm_handler;
	interrupt.sa_handler = &interrupt_handler;
	sigaction(SIGALRM, &alarm, NULL);
	sigaction(SIGINT, &interrupt, NULL);
	sigaction(SIGTERM, &interrupt, NULL);
	sigaction(SIGKILL, &interrupt, NULL);
        init_server(&server_socket, &server_addr, port);
        while(server_running){
            usleep(50);//to prevent cpu overheat
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
                printf("THE GAME IS RUNNING \n"); //ONLY FOR DEBUG
            }
        }
        if(argc==3)
            fclose(fpError);
        if(close(fdLock)==-1)
            perror("Error closing lock file\n");
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
		if (game_running || amount_players == MAX_PLAYERS) {
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
            reset_player(players[i]);
        }
        amount_players=0;
}
void reset_player(player pl){
    pl.socket=0;
    pl.name[0]='\0';
    pl.is_registered=FALSE;
}

void shutdown_server() {
        sprintf(mess.payload,"The server has shut down\n");
        mess.code=C_SERVER_SHUT_DOWN;
        send_message_everybody(mess);
	printf("server shutting down ..\n");
	clear_lobby();
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
    reset_player(players[idx_player]);
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
    shutdown_socket(server_socket);
    shutdown_server();
}
void start_game() {
    printf("A game is starting with %i players.\n", amount_players);
    sprintf(mess.payload,"The Game starts now !\n Amount of players in the game : %i\n May the best win !\n", amount_players);
    send_message_everybody(mess);
    game_running = TRUE;
    start_round();
}


void start_round() {
    deal_cards();
    //TO DO
}

void deal_cards() {
    //TO DO
}

void send_message_everybody(message msg){
    for(int i=0; i < amount_players; i++){
        send_message(msg, players[i].socket);
    }
}


