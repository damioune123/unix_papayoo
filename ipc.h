/**
*
*  AUTHORS : MANIET Alexandre (amaniet15)(serie 2) , MEUR Damien (dmeur15)(serie 2)
*  This file contains the header and constant necessary for the ipc handling
*
*/
#ifndef IPC_H
#define IPC_H
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "utils.h"
#define MAX_PLAYERS 4
#define MIN_PLAYERS 2
#define BUFFER_SIZE 255
typedef struct s_mem{
	int scores[MAX_PLAYERS];
	char names[MAX_PLAYERS][BUFFER_SIZE];
} s_mem;
#define TOKEN_MUT_MEM 137
#define TOKEN_MUT_RC 138
#define TOKEN_SEG_MEM 139
#define TOKEN_SEG_RC 140

void s_read_scores( int **);//used for filling the int array with the scores in shared memory
void s_read_names(char **);//used for filling the char * array with the player's names in shared memory
void s_write_score( int, int );// used to write a player's score to his own index in the array of scores (corresponding to the index in the players array in server
void s_write_name( int, char* );//used to to write a player's name to his own index in the array of names (corresponding to the index in the player's array in server
void create_segment();//used to create the segment of shared memory (server only)
void locate_segment();//used to locate the segment of shared memory (player only)
void init_semaphores();//used to create the semaphores (server only)
void locate_semaphores();//used lo locate the semaphores (player only)
void down(int);//down operation on a semaphore 
void up(int);//up operation on a semaphore
void kill_ipcs();//kill all ipcs (semapohres and shared memory)
#endif