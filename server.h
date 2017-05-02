#ifndef SERVER_H
#define SERVER_H
#include "utils.h"

#define EXIT_ERROR 1
#define EXI_SUCCESS 0
#define MAX_PLAYERS 4

typedef struct player {
	int socket;
        struct sockaddr_in client_addr;
	char name[255];
} player;

#endif

void add_player(player * , int);
void init_server(int *, struct sockaddr_in *);
