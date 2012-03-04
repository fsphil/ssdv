
CC=gcc
CFLAGS=-g -Wall
LDFLAGS=-g -lm

ssdv: main.o ssdv.o rs8.o ssdv.h rs8.h
	$(CC) $(LDFLAGS) main.o ssdv.o rs8.o -o ssdv

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm *.o

