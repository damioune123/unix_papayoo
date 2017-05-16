/**
*
*  AUTHORS : MANIET Alexandre (amaniet15)(serie 2) , MEUR Damien (dmeur15)(serie2)
*  This file contains all the necesary structures, functions declarations and constant needed by the server program.
*
*/
#ifndef SERVEUR_H
#define SERVEUR_H
#include "socket.h"

#define COUNTDOWN 10 //Countdown to wait to start the game during lobby registration
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
boolean all_players_registered();//return TRUE if all connected client are connected, FALSE else
void remove_player(int); //removes a player from the game
void start_game(); //starts the game
void deal_cards(); //shuffles the deck and deals cards to all players
void start_round(); //starts a new round
void init_shared_memory();//used to put the players names and reset scores and cards in shared memory
void init_deck();//used to initialize the whole deck
void shuffle_deck();//used to shuffle the whole deck
card add_card(card);//used to add a card in the whole deck during initialization
void show_cards(card *, int);//used to show cards : debug only (this function is used only client side)
void show_card(card cardToShow, char *);//used to show a single card : debu only (this function is only client side)
void find_papayoo();//used to find the type of the papayoo for the next round (randomly) and notificate all the players once chosen
void receive_ecart_from_player(int, message); //used to receive an ecart from a player
void send_ecart_back();//send ecart to player after all ecarts have been received
void send_basic_info();//send the basic info to a player (amount_players, player index, papayoo)
void ask_for_card(int);//ask a player to play a card
void receive_played_card(int, message); //receive a played card from a player
void end_turn();//finds out the looser of the pli and sends him the cards of the pli
void send_pli(int); //This function sends the pli to the player who lost the turn
void end_round();//ends a round , asks all players to send their score
void update_score(int, message);//update score in shared memory for a single player
#endif
