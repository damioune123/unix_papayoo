#ifndef S_MEMORY_H
#define S_MEMORY_H
#include <sys/types.h>
#include<stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#define SHMSZ     27
void * create_segment();
void * locate_segment();
#endif
