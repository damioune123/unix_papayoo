#ifndef SERVER_H
#define SERVER_H
#include "utils.h"
#include<stdlib.h>


#define MAX_PLAYERS 4

typedef struct player {
	int socket;
	char name[255];
} player;
#endif
