/**
*
*  AUTHORS : MANIET Alexandre (amaniet15)(serie 2) , MEUR Damien (dmeur15)(serie2)
*  This file contains all the necesary structures, functions declarations and constant needed by the server program.
*
*/
#ifndef SERVEUR_H
#define SERVEUR_H
#include "socket.h"

#define COUNTDOWN 10 
#define SERVER_LOCK "./server.lock"
typedef void (*fct_ptr)( );
typedef struct player {
	int socket;
        int is_registered;
	char name[255];
} player;

int find_player_id_by_socket(int);//return -1 if no player found
void reset_players();//reset the value of all the players
void reset_player(player*); //reset the value of a particuliar player
void alarm_handler(int); //handles alarm timeouts
void interrupt_handler(int); //shuts down the server when a SIGINT, SIGKILL OR SIGTERM  occurs
void shutdown_server(); //halts the server
void send_message_everybody(message);//send message to all players
void clear_lobby();//clear the lobby
void add_client(int, struct sockaddr_in*); //adds a client to the fdset
void add_player(int, message); //confirm connection and inform client
int all_players_registered();//return TRUE if all connected client are connected, FALSE else
void remove_player(int); //removes a player from the game
void start_game(); //starts the game
void deal_cards(); //shuffles the deck and deals cards to all players
void start_round(); //starts a new round
void init_shared_memory();//used to put the players names and reset scores and cards in shared memory
void init_deck();
void shuffle_deck();
card pick_card();
card add_card(card);
void show_cards(card *, int);
void show_card(card cardToShow, char *);
#endif
