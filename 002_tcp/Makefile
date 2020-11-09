CC=gcc
CFLAGS=-Wall -g -c -O2

all:server client

server:server.o
	$(CC) $< -o $@

client:client.o
	$(CC) $< -o $@

%*.o:%*.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY:clean

clean:
	$(RM) *.o server client a.out -rf
