all: list_mod.cgi imod.cgi rmod.cgi

list_mod.cgi: list_mod.o
	gcc -pedantic -Wall -Werror -O3 -o list_mod.cgi list_mod.o

list_mod.o: list_mod.c
	gcc -pedantic -Wall -Werror -c list_mod.c

imod.cgi: imod.o
	gcc -pedantic -Wall -Werror -O3 -o imod.cgi imod.o

imod.o: imod.c
	gcc -pedantic -Wall -Werror -c imod.c

rmod.cgi: rmod.o
	gcc -pedantic -Wall -Werror -O3 -o rmod.cgi rmod.o

rmod.o: rmod.c
	gcc -pedantic -Wall -Werror -c rmod.c

clean:
	rm -f *.o list_mod.cgi
	rm -f *.o imod.cgi
	rm -f *.o rmod.cgi
