all: servidor 
servidor: sock_servidor_u.o
	   gcc -pedantic -Wall -Werror -o servidor sock_servidor_u.o

sock_servidor_u.o:  sock_servidor_u.c 
	   gcc -pedantic -Wall -Werror -c sock_servidor_u.c 

clean:
	rm -f *.o servidor



