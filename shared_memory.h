#ifndef S_MEMORY_H
typedef struct s_mem{
	int scores[4];
	char names[4][255];
	// TODO add more fields to the share memory structure
} s_mem;
#define S_MEMORY_H
#include <sys/types.h>
#include<stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#define SHMSZ     27
#define CODE_READ_NAMES		1
#define CODE_READ_SCORE		2
#define CODE_READ_CURRENT_CARDS	3
void * create_segment();
void * locate_segment();
#endif
