#include "server.h"

player players[MAX_PLAYERS];
int port;
FILE *fpError;
int server_running, game_running, amount_players;
int stock_addr_size = sizeof(struct sockaddr_in);
message mess;
struct timeval timeout = {0, 200000};//time to wait to recv essage before cancelling the operation (here 200 ms)
int main(int argc , char *argv[])
{
        int i, max_fd, server_socket, select_res;
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
        for( i=0; i < MAX_PLAYERS; i++){
            players[i].socket=0;
        }
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
        init_server(&server_socket, &server_addr);
        while(server_running){
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
                //TO DO
            }
        }
        if(argc==3)
            fclose(fpError);
        shutdown_socket(server_socket);
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
        strcpy(players[idx_player].name, mesRecv.payload);
        mess.code=C_OK;
        strcpy(mess.payload, M_SIGNUP_CLIENT_OK);
	if (amount_players == 1) {
		alarm(COUNTDOWN);
	}
        send_message(mess, socket);
        printf("Player %s has joined the lobby\n", players[idx_player].name);
}
int find_player_id_by_socket(int socket){
    for(int j = 0; j < MAX_PLAYERS; j++){
        if(players[j].socket == socket)
                return j;
    }
    return -1;
}

void init_server(int *server_socket, struct sockaddr_in *server_addr) {

	if ((*server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error creating the socket");
		exit(EXIT_FAILURE);
	}

	printf("Socket created\n");

	//Prepare the sockaddr_in structure
        server_addr->sin_family = AF_INET;
	server_addr->sin_addr.s_addr = INADDR_ANY;
	server_addr->sin_port = htons(port);

	//Bind
	if( bind(*server_socket,(struct sockaddr *)server_addr , sizeof(*server_addr)) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		exit(EXIT_FAILURE);
	}
        //Listen
	listen(*server_socket , MAX_PLAYERS);
}



void shutdown_server() {
	printf("server shutting down ..\n");
	clear_lobby();
        server_running = FALSE;
}



void clear_lobby() {
    for(int i=0; i < amount_players; i++){
        shutdown_socket(players[i].socket);
    }
    amount_players=0;
    printf("Enter clear lobby method \n");
}


void remove_player( int socket) {
    int idx_player = find_player_id_by_socket(socket);
    char namePl[255];
    if(idx_player ==-1){
        fprintf(stderr, "The server couldn't bind the socket with a player\n");
        return;
    }
    shutdown_socket(players[idx_player].socket);
    players[idx_player].socket=0;
    strcpy(namePl, players[idx_player].name);
    for(int j=idx_player;j< amount_players; j++ ){
        players[j]=players[j+1];
    }
    amount_players--;
    printf("The player %s has been successfully removed from the game \n", namePl);
    sprintf(mess.payload,"The player %s has left  the game\n", namePl);
    mess.code=C_INFO;
    send_message_everybody(mess);

}

void send_message_everybody(message msg){
    for(int i=0; i < amount_players; i++){
        send_message(msg, players[i].socket);
    }
}
void alarm_handler(int signum) {
    if (signum == SIGALRM) {
        if (amount_players < 2) {
            clear_lobby();
        } else {
            start_game();
        }
    }
}
void interrupt_handler(int signum) {
    if (signum == SIGINT) {
        shutdown_server();
    }
}
void start_game() {
    start_round();
    game_running = TRUE;
}

void start_round() {
    deal_cards();
    //TO DO
}

void deal_cards() {
    //TO DO
}


