CC = gcc
CFLAGS = -Wall -O2 -g

.PHONY: all
all: client server

client: client.c
	$(CC) $(CFLAGS) -o client client.c

server: server.c
	$(CC) $(CFLAGS) -o server server.c

.PHONY: clean
clean:
	rm -f client server
