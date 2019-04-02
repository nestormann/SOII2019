all: servidor 
servidor: sock_servidor_i.o
	   gcc -pedantic -Wall -Werror -o servidor sock_servidor_i.o

sock_servidor_i.o:  sock_servidor_i.c
	   gcc -pedantic -Wall -Werror -c sock_servidor_i.c 

clean:
	rm -f *.o servidor




