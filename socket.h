/**
*
*  AUTHORS : MANIET Alexandre (amaniet15)(serie 2) , MEUR Damien (dmeur15)(serie 2)
*  This file contains all the necesary structures, functions declarations and constant needed by both the server and the player program.
*
*/
#include "utils.h"
#define TIME_TRY_CONNECT 5 //time in seconds to wait to connect to server if connection had been refused
#ifndef SOCKET_H
#define SOCKET_H
void init_server(int *, struct sockaddr_in *, int);//initialize the server 
void connect_to_server(int*, struct sockaddr_in *, char *, int);//allows the client to connect to the server
int receive_message(message*, int);//allow either the client or the server to receive a message structure through socket
void send_message(message, int);//allow either the client or the server to send a message structure through socket
void shutdown_socket(int);//close a socket
#endif
