.DEFAULT_GOAL := all

CC=gcc
CFLAGS=-Wall

server:
	$(CC) $(CFLAGS) -o server server.cpp
client:
	$(CC) $(CFLAGS) -o client client.cpp
all: server client

clean:
	rm -f server client
