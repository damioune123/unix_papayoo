/**
*
*  AUTHORS : MANIET Alexandre (amaniet2015), MEUR Damien (dmeur2015)
*  This file contains all the necessary constants and functions declarations needed by both client and server programs
*
*/
#ifndef UTIL_H
#define UTIL_H
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#include <signal.h>
#include <errno.h>
#include <sys/select.h>
#include <signal.h>
#include "message.h"
#define TRUE	1
#define FALSE	0
int receive_message(message *, int);
void send_message(message, int);
void shutdown_socket(int);
#endif
