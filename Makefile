
# AUTHORS : MANIET Alexandre, MEUR Damien

########################################
# Makefile  : v0.1
#######################################


CFLAGS=-g

all : server client


server : server.o utils.o
	cc $(CFLAGS) -o server server.o utils.o

client : client.o utils.o
	cc $(CFLAGS) -o client client.o utils.o

server.o : server.c server.h utils.h 
	cc $(CFLAGS) -c server.c

client.o : client.c client.h utils.h 
	cc $(CFLAGS) -c client.c

util.o : utils.c utils.h
	cc $(CFLAGS) -c utils.c
clean :
	rm *.o
	rm server
	rm client

