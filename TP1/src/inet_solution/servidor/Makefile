all: servidor firmware.bin
servidor: sock_servidor_i.o
	gcc -pedantic -Wall -Werror -o servidor sock_servidor_i.o
	   
firmware.bin: sock_cliente_i.o 
ifeq ($(CC),arm)
	arm-linux-gnueabi-gcc -pedantic -Wall -Werror -o firmware.bin sock_cliente_i.o
else
	gcc -pedantic -Wall -Werror -o firmware.bin sock_cliente_i.o
endif
sock_servidor_i.o:  sock_servidor_i.c
	gcc -pedantic -Wall -Werror -c sock_servidor_i.c 

sock_cliente_i.o: sock_cliente_i.c 
ifeq ($(CC),arm)
	arm-linux-gnueabi-gcc -pedantic -Wall -Werror -c sock_cliente_i.c 
else
	gcc -pedantic -Wall -Werror -c sock_cliente_i.c 
endif

clean:
	rm -f *.o servidor
	rm -f *.o firmware.bin

