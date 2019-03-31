#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <sys/types.h> 
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/wait.h>
#include <errno.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <dirent.h>
#include <getopt.h>

#define TAM 4096

int main(int argc, char *argv[]) 
 {
	int sockfd , n;
	socklen_t clilen, servlen;
	struct sockaddr_un  cli_addr, serv_addr;
	int user_verification=0;
	int pass_verification=0;
	char usuario[]="nestor";
	char password[]="parapam";
	char current_directory[100];
	char *host= (char*) malloc(1024); 
	gethostname(host, 1024);
	char prompt[TAM];
	sprintf(prompt, "%s@%s:~%s$ ",usuario,host,getcwd(current_directory,100));
	char buffer[TAM];

    /* Se toma el nombre del socket de la línea de comandos */
    if( argc != 2 ) 
	 {
            printf( "Uso: %s <nombre_de_socket>\n", argv[0] );
            exit( 1 );
     }

	if ( ( sockfd = socket( AF_UNIX, SOCK_STREAM, 0) ) < 0 ) 
	 {
		perror( "creación de  socket");
		exit(1);
	 }

	/* Remover el nombre de archivo si existe */
	unlink ( argv[1] );

	memset( &serv_addr, 0, sizeof(serv_addr) );
	serv_addr.sun_family = AF_UNIX;
	strcpy( serv_addr.sun_path, argv[1] );
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if( bind( sockfd,(struct sockaddr *)&serv_addr,servlen )<0 ) 
	 {
		perror( "ligadura" ); 
		exit(1);
	 }

    printf( "Proceso: %d - socket disponible: %s\n", getpid(), serv_addr.sun_path );

	listen( sockfd, 5 );
	clilen = sizeof( cli_addr );

	while ( 1 ) 
	 {	int newsockfd, pid;
		newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, &clilen );
		if ( newsockfd < 0 ) 
		 {
			perror( "accept" );
			exit( 1 );
		 }

		pid = fork();
		if ( pid < 0 ) 
		 {
			perror( "fork" );
			exit( 1 );
		 }

		/*Proceso que atendera al cliente*/
		if ( pid == 0 ) 
		 {
			close( sockfd );

			while(1) 
			 { 	/*Verificacion del usuario*/
			 	while(user_verification==0)
				 {										
					memset( buffer, 0, TAM );
					n = read( newsockfd, buffer, TAM-1 );
					if ( n < 0 ) 
					 {
						perror( "lectura de socket" );
						exit(1);
					 }
					user_verification=strncmp(buffer,usuario,strlen(usuario));
					if(user_verification!=0)
					 {
						printf("usuario incorrecto, matando proceso %d\n",getpid());
						n = write( newsockfd, "wrongUser", 10 );
						if ( n < 0 ) 
						 {
							perror( "escritura en socket" );
							exit( 1 );
						 }
						exit(0);
					 }
					else
					 {
						printf("Usuario correcto\n");
						n = write( newsockfd, "correctUser", 12);
						if ( n < 0 ) 
						 {
							perror( "escritura en socket" );
							exit( 1 );
						 }
						user_verification=1;
					 }
				 }

				/*Verificacion de la contraseña*/
				while(pass_verification==0)
				 {											
					printf("Espera de contraseña\n");
					memset( buffer, 0, TAM );
					n = read( newsockfd, buffer, TAM-1 );
					if ( n < 0 ) 
					 {
						perror( "lectura de socket" );
						exit(1);
					 }

					if(strncmp(buffer,password,strlen(buffer)-1)!=0)
					 {
						printf("Contraseña incorrecta, reintento\n");
						n = write( newsockfd, "wrongPass", 10 );
						if ( n < 0 ) 
						 {
							perror( "escritura en socket" );
							exit( 1 );
						 }
					 }
					else
					 {
						printf("Contraseña correcta, el cliente se ha conectado exitosamente\n");
						n = write( newsockfd, "correctPass", 12);
						if ( n < 0 ) 
						 {
							perror( "escritura en socket" );
							exit( 1 );
						 }
						pass_verification=1;

					 }
				 }
				
				while(1) 
				 { 
				 	/*Recibo mensaje de SINC*/
					memset( buffer, 0, TAM );
 					n = read( newsockfd, buffer, TAM-1 );
                    if ( n < 0 ) 
                     {
						perror( "lectura de socket" );
   		                exit(1);
                	 }

                	/*Envio el PROMPT*/
		            n = write( newsockfd, prompt, strlen(prompt) );
					if ( n < 0 ) 
					 {
						perror( "escritura en socket" );
						exit( 1 );
					 }

		            /*Espero por comando*/
		            memset( buffer, 0, TAM );
 				 	n = read( newsockfd, buffer, TAM-1 );
                    if ( n < 0 ) 
                     {
						perror( "lectura de socket" );
   		                exit(1);
                	 }
                    printf( "Recibí el siguiente comando: %s", buffer );

                    /*Respuesta del comando*/
					n = write( newsockfd, "Ejecute", 8 );
					if ( n < 0 ) 
					 {
						perror( "escritura en socket" );
						exit( 1 );
					 }
		            
		            /*Verificacion de fin*/
					buffer[strlen(buffer)-1] = '\0';
					if( !strcmp( "fin", buffer ) ) 
					 {
						printf( "PROCESO %d. Como recibí 'fin', termino la ejecución.\n\n", getpid() );
						exit(0);
					 }
				 }				
			 }
		 }
		else 
		 {
			printf( "SERVIDOR: Nuevo cliente, que atiende el proceso hijo: %d\n", pid );
			close( newsockfd );
		 }
	 }
	return 0; 
 } 
