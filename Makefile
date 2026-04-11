CC = gcc
CFLAGS = -O3
LIBS = -lncurses

default: lectura1

all: lectura1

lectura1: lectura1.c Makefile
	$(CC) $(CFLAGS) -o lectura1 lectura1.c $(LIBS)

clean:
	-rm -f lectura1