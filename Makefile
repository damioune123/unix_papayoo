
# AUTHORS : MANIET Alexandre, MEUR Damien
# amaniet2015, dmeur15
########################################
# Makefile  : v0.1
#######################################


CFLAGS=-g

all : serveur joueur


serveur : serveur.o socket.o ipc.o
	cc $(CFLAGS) -o serveur serveur.o socket.o ipc.o

joueur : joueur.o socket.o ipc.o
	cc $(CFLAGS) -o joueur joueur.o socket.o ipc.o

serveur.o : serveur.c serveur.h socket.h utils.h ipc.h cards.h message.h
	cc $(CFLAGS) -c serveur.c

joueur.o : joueur.c joueur.h socket.h utils.h ipc.h cards.h message.h
	cc $(CFLAGS) -c joueur.c

socket.o : socket.c socket.h utils.h cards.h message.h
	cc $(CFLAGS) -c socket.c

ipc.o : ipc.c ipc.h cards.h message.h
	cc $(CFLAGS) -c ipc.c
clean :
	rm *.o
	rm serveur
	rm joueur

