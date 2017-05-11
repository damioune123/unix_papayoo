#ifndef S_MEMORY_H
#define S_MEMORY_H
#include <sys/types.h>
#include<stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
#include <sys/sem.h>
typedef struct s_mem{
	int scores[4];
	char names[4][255];
	// TODO add more fields to the share memory structure
} s_mem;
#define TOKEN_MUT_MEM 137
#define TOKEN_MUT_RC 138
#define TOKEN_SEG_MEM 139
#define TOKEN_SEG_RC 140

void s_read_scores( int **);
void s_read_names(char **);
void s_write_score( int, int );
void s_write_name( int, char* );
void  create_segment();
void  locate_segment();
void init_semaphores();
void locate_semaphores();
void down(int);
void up(int);
#endif
