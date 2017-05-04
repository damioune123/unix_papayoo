#ifndef SERVER_H
#define SERVER_H
#include "utils.h"
#include <sys/select.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>


#define EXIT_ERROR 1
#define EXI_SUCCESS 0
#define MIN_PLAYERS 2
#define MAX_PLAYERS 4
#define COUNTDOWN 30 //30 seconds te
typedef void (*fct_ptr)( );
typedef struct player {
	int socket;
        struct sockaddr_in client_addr;
	char name[255];
} player;

#endif
int find_player_id_by_socket(int);//return -1 if no player found
void init_server(int *, struct sockaddr_in *);
void alarm_handler(int); //handles alarm timeouts
void interrupt_handler(int); //shuts down the server when a SIGINT occurs
void shutdown_socket(int); //closes a given socket
void shutdown_server(); //halts the server
void add_client(int, struct sockaddr_in*); //adds a client to the fdset
void add_player(int, message); //confirm connection and inform client
void remove_player(player*, int, int); //removes a player from the game
int receive_msg(message*, int); //handles incoming messages
void send_message(message, int);
void start_game(); //starts the game
void deal_cards(); //shuffles the deck and deals cards to all players
void start_round(); //starts a new round
