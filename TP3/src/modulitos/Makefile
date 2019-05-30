all: list_mod.cgi

list_mod.cgi: list_mod.o
	gcc -pedantic -Wall -Werror -O3 -o list_mod.cgi list_mod.o

list_mod.o: list_mod.c
	gcc -pedantic -Wall -Werror -c list_mod.c

clean:
	rm -f *.o list_mod.cgi
