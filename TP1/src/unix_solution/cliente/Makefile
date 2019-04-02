all: cliente 
cliente: sock_cliente_u.o 
	   gcc -pedantic -Wall -Werror -o cliente sock_cliente_u.o 

sock_cliente_u.o:     sock_cliente_u.c 
	   gcc -pedantic -Wall -Werror -c sock_cliente_u.c 

clean:
	rm -f *.o cliente




