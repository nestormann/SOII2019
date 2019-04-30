all: cliente 
ifeq ($(CC),arm)
cliente: sock_cliente_i.o 
	   arm-linux-gnueabi-gcc -pedantic -Wall -Werror -o cliente sock_cliente_i.o 

sock_cliente_i.o: sock_cliente_i.c 
	   arm-linux-gnueabi-gcc -pedantic -Wall -Werror -c sock_cliente_i.c 
else
cliente: sock_cliente_i.o 
	   gcc -pedantic -Wall -Werror -o cliente sock_cliente_i.o 

sock_cliente_i.o: sock_cliente_i.c 
	   gcc -pedantic -Wall -Werror -c sock_cliente_i.c 
endif
clean:
	rm -f *.o cliente




