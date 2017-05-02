########################################
# Makefile  : v0.1
#######################################

CFLAGS=-g

all : server client

server : server.o
	cc $(CFLAGS) -o server server.o

client : client.o
	cc $(CFLAGS) -o client client.o

server.o : server.c server.h
	cc $(CFLAGS) -c server.c

client.o : client.c client.h
	cc $(CFLAGS) -c client.c

clean :
	rm *.o
	rm server
	rm client

