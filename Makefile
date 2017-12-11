.DEFAULT_GOAL := all

CC=g++
CFLAGS=-Wall

server:
	$(CC) $(CFLAGS) -o server server.cpp
client:
	$(CC) $(CFLAGS) -o client client.cpp
all: server client

clean:
	rm -f server client
