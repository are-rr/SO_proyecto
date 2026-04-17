CC = gcc
CFLAGS = -Wall -Wextra
LIBS = -lncurses

OBJ = main.o operaciones.o listas.o validaciones.o nc.o otros.o

programa: $(OBJ)
	$(CC) $(OBJ) -o programa $(LIBS)

main.o: main.c procesos.h
	$(CC) $(CFLAGS) -c main.c

operaciones.o: operaciones.c procesos.h
	$(CC) $(CFLAGS) -c operaciones.c

listas.o: listas.c procesos.h
	$(CC) $(CFLAGS) -c listas.c

validaciones.o: validaciones.c procesos.h
	$(CC) $(CFLAGS) -c validaciones.c

nc.o: nc.c procesos.h
	$(CC) $(CFLAGS) -c nc.c

otros.o: otros.c procesos.h
	$(CC) $(CFLAGS) -c otros.c

run: programa
	./programa

clean:
	rm -f *.o programa