/**
*
*  AUTHORS : MANIET Alexandre (amaniet2015)(serie 2), MEUR Damien (dmeur15)(serie 2)
*  This file contains the structure and constants needed to communicate between the server and client through sockets.
*
*/
#include "cards.h"
#ifndef MESSAGE_H
#define MESSAGE_H
#define MESSAGE_MAX_LENGTH 512
typedef struct basic_info{
    int player_index;
    int amount_players;
    card_const papayoo;
    int current_round;
} basic_info;
typedef struct message {
    int code;
    char payload[MESSAGE_MAX_LENGTH];
    card deck[DECK_PHYSICAL_SIZE/2];
    int deck_logical_size;
    basic_info info;
} message;
//server-> client
//code
#define C_OK 0
#define C_REFUSE 1
#define C_SERVER_ERROR 2
#define C_INFO 3
#define C_GAME_CANCELLED 4
#define C_SERVER_SHUT_DOWN 5
#define C_INIT_DECK_RECEIVED 6
#define C_ALL_ECART_DECK_RECEIVED 7
#define C_BASIC_INFO 8
#define C_ASK_FOR_CARD 9
#define C_SHOW_PLI 10
#define C_ADD_PLI 11

//message
#define M_SERVER_ERROR "An error occured on ther servor\n"
#define M_CONN_REFUSE "Sorry the connection was refused by the server\n"
#define M_GREET_CLIENT "Welcome to Papayoo Online, you are now connected\n" 
#define M_SIGNUP_CLIENT_OK "Thank you ! You've been correctly signed up for the next game.\n" 
//client-> server
#define C_DEFAULT 1337 
#define C_ADD_PLAYER 0
#define C_ECART_DECK_SENT 1
#define C_PLAY_CARD 2
#endif
