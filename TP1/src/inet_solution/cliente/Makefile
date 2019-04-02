all: cliente 
cliente: sock_cliente_i.o 
	   gcc -pedantic -Wall -Werror -o cliente sock_cliente_i.o 

sock_cliente_i.o:     sock_cliente_i.c 
	   gcc -pedantic -Wall -Werror -c sock_cliente_i.c 

clean:
	rm -f *.o cliente




