CC = gcc
CFLAGS = -g

all: client server

fork: client.c
	$(CC) $(CFLAGS) -o client client.c

exec: server.c
	$(CC) $(CFLAGS) -o server server.c
