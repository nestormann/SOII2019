all: servidor firmware.bin
servidor: sock_servidor_i.o
	   gcc -pedantic -Wall -Werror -o servidor sock_servidor_i.o
	   
firmware.bin: sock_cliente_i.o 
	   gcc -pedantic -Wall -Werror -o firmware.bin sock_cliente_i.o 

sock_servidor_i.o:  sock_servidor_i.c
	   gcc -pedantic -Wall -Werror -c sock_servidor_i.c 

sock_cliente_i.o:     sock_cliente_i.c 
	   gcc -pedantic -Wall -Werror -c sock_cliente_i.c 

clean:
	rm -f *.o servidor
	rm -f *.o firmware.bin




