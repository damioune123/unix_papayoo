/**
*
*  AUTHORS : MANIET Alexandre, MEUR Damien
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
