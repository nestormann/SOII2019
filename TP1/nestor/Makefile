all: cliente servidor
cliente: sock_cliente_u.o 
	   gcc -pedantic -Wall -Werror -o cliente sock_cliente_u.o 

servidor: sock_servidor_u.o
	   gcc -pedantic -Wall -Werror -o servidor sock_servidor_u.o

sock_cliente_u.o:     sock_cliente_u.c 
	   gcc -pedantic -Wall -Werror -c sock_cliente_u.c 

sock_servidor_u.o:  sock_servidor_u.c
	   gcc -pedantic -Wall -Werror -c sock_servidor_u.c 

clean:
	rm -f *.o cliente
	rm -f *.o servidor




