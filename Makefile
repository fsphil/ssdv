
CC=gcc
CFLAGS=-g -Wall
LDFLAGS=-g

ssdv: main.o ssdv.o rs8.o
	$(CC) $(LDFLAGS) main.o ssdv.o rs8.o -o ssdv

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm *.o

