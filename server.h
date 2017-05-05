/**
*
*  AUTHORS : MANIET Alexandre (amaniet15) , MEUR Damien (dmeur15)
*  This file contains all the necesary structures, functions declarations and constant needed by the server program.
*
*/
#ifndef SERVER_H
#define SERVER_H
#include "utils.h"

#define MIN_PLAYERS 2
#define MAX_PLAYERS 4
#define COUNTDOWN 10 
#define SERVER_LOCK "./server.lock"
typedef void (*fct_ptr)( );
typedef struct player {
	int socket;
        int is_registered;
	char name[255];
} player;

int find_player_id_by_socket(int);//return -1 if no player found
void init_server(int *, struct sockaddr_in *);//initialize the server 
void reset_players();//reset the value of all the players
void reset_player(); //reset the value of a particuliar player
void alarm_handler(int); //handles alarm timeouts
void interrupt_handler(int); //shuts down the server when a SIGINT, SIGKILL OR SIGTERM  occurs
void shutdown_server(); //halts the server
void send_message_everybody(message);//send message to all players
void clear_lobby();//clear the lobby
void add_client(int, struct sockaddr_in*); //adds a client to the fdset
void add_player(int, message); //confirm connection and inform client
int all_players_registered();
void remove_player(int); //removes a player from the game
void start_game(); //starts the game
void deal_cards(); //shuffles the deck and deals cards to all players
void start_round(); //starts a new round
#endif
