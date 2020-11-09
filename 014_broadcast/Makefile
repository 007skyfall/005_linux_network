CC=gcc
CFLAGS=-Wall -g -c -O2

all:recv send

recv:recv.o
	$(CC) $< -o $@

send:send.o
	$(CC) $< -o $@

%*.o:%*.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY:clean

clean:
	$(RM) *.o recv send a.out
