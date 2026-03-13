lectura1: lectura1.c
	gcc lectura1.c -o lectura1 -lncurses

run: lectura1
	./lectura1

clean:
	rm -f lectura1