#ifndef S_MEMORY_H
#define S_MEMORY_H
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

void s_read_scores( int **);
void s_read_names(char **);
void s_write_score( int, int );
void s_write_name( int, char* );
void create_segment();
void locate_segment();
void init_semaphores();
void locate_semaphores();
void down(int);
void up(int);
void kill_ipcs();
#endif
