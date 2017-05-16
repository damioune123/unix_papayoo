/**
*
*  AUTHORS : MANIET Alexandre (amaniet2015)(serie 2), MEUR Damien (dmeur2015)(serie 2)
*  This file contains all the miscelleneous constants and imports needed by both client and server programs
*
*/
#ifndef UTIL_H
#define UTIL_H
#include <limits.h>
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
#include <time.h>
#include "message.h"
#include "ipc.h"
#include "cards.h"
typedef enum boolean {FALSE, TRUE} boolean;
#endif
