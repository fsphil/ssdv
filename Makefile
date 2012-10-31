
CC=gcc
CFLAGS=-g -O3 -Wall
LDFLAGS=-g

all: ssdv

ssdv: main.o ssdv.o rs8.o ssdv.h rs8.h
	$(CC) $(LDFLAGS) main.o ssdv.o rs8.o -o ssdv

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

install: all
	mkdir -p ${DESTDIR}/usr/bin
	install -m 755 ssdv ${DESTDIR}/usr/bin

clean:
	rm -f *.o ssdv

