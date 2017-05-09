
# AUTHORS : MANIET Alexandre, MEUR Damien
# amaniet2015, dmeur15
########################################
# Makefile  : v0.1
#######################################


CFLAGS=-g

all : serveur joueur


serveur : serveur.o socket.o shared_memory.o
	cc $(CFLAGS) -o serveur serveur.o socket.o shared_memory.o

joueur : joueur.o socket.o shared_memory.o
	cc $(CFLAGS) -o joueur joueur.o socket.o shared_memory.o

serveur.o : serveur.c serveur.h socket.h utils.h shared_memory.h
	cc $(CFLAGS) -c serveur.c

joueur.o : joueur.c joueur.h socket.h utils.h shared_memory.h
	cc $(CFLAGS) -c joueur.c

socket.o : socket.c socket.h utils.h
	cc $(CFLAGS) -c socket.c

shared_memory.o : shared_memory.c shared_memory.h
	cc $(CFLAGS) -c shared_memory.c
clean :
	rm *.o
	rm serveur
	rm joueur

