/**
*
*  AUTHORS : MANIET Alexandre (amaniet15) , MEUR Damien (dmeur15)
*  This fille content the header and constant necessary for the client program
*
*/

#include "utils.h"
#ifndef CLIENT_H
#define CLIENT_H
#define TIME_TRY_CONNECT 5 //time in seconds to wait to connect to server if connection had been refused
void try_to_connect(int *, struct sockaddr_in *);
void connect_to_server(int*, struct sockaddr_in *);
void signup(int *);
#endif
