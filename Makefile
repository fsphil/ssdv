PROJECT=ssdv
CC=gcc
CFLAGS=-g -O3 -Wall
LDFLAGS=-g
INSTALL=install
RM=rm -f
OBJS=src/main.o src/ssdv.o src/rs8.o
HEADS=src/ssdv.h src/rs8.h

all: $(PROJECT)

$(PROJECT): $(OBJS) $(HEADS)
	$(CC) $(LDFLAGS) $^ -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

install: all
	$(INSTALL) -d ${DESTDIR}/usr/bin
	$(INSTALL) -m 755 $(PROJECT) ${DESTDIR}/usr/bin

clean:
	$(RM) $(OBJS) $(PROJECT)

