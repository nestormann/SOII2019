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
	char file_to_send[100]="";
	int enviar = 0;
	char *token;
	int fd;
	struct stat file_stat;
	int sent_bytes = 0;
	off_t offset;
	ssize_t len;
	char file_size[256];
	int remain_data;

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
				
				/*Procedimiento de descarga*/
				if(enviar==1)
					{	
					printf("%s\n",file_to_send);
					fd = open(file_to_send, O_RDONLY);
					if (fd == -1)
						{
						fprintf(stderr, "Error opening file --> %s", strerror(errno));
						n = write( newsockfd,"nofile" , 6);		
						if ( n < 0 ) 
							{
							perror( "escritura en socket" );
							exit( 1 );
							}
						enviar=0;
						}
					else
						{ 
						if (fstat(fd, &file_stat) < 0)
							{
							fprintf(stderr, "Error fstat --> %s", strerror(errno));
							exit(EXIT_FAILURE);
							}

						fprintf(stdout, "File Size: \n%zd bytes\n", file_stat.st_size);

						clilen = sizeof(struct sockaddr_in);

						/* Sending file size */
						sprintf(file_size, "%zd", file_stat.st_size);
						len = send(newsockfd, file_size, sizeof(file_size), 0);

						if (len < 0)
							{
							fprintf(stderr, "Error on sending greetings --> %s", strerror(errno));
							exit(EXIT_FAILURE);
							}

						fprintf(stdout, "Server sent %zd bytes for the size\n", len);

						offset = 0;
						remain_data = file_stat.st_size;

						/* Sending file data */
						while (((sent_bytes = sendfile(newsockfd, fd, &offset, TAM)) > 0) && (remain_data >= 0) )
							{
							remain_data -= sent_bytes;
							}

						printf("FINALIZO DESCARGA\n");
						/*Espera confirmacion para finalizar la descarga*/
						memset( buffer, 0, TAM );
						n = read( newsockfd, buffer, TAM-1 );
						if ( n < 0 ) 
							{
							perror( "lectura de socket" );
							exit(1);
							}

						if(strcmp("done", buffer)==0) 
							enviar=0;	
						}
					}

				if(enviar==0)
					{
					/*Recibo mensaje de SINC*/
					memset( buffer, 0, TAM );
					n = read( newsockfd, buffer, TAM-1 );
					if ( n < 0 ) 
						{
						perror( "lectura de socket" );
						exit(1);
						}
					}
				
				if(enviar==0)
				{
					/*Envio el PROMPT*/
					n = write( newsockfd, prompt, strlen(prompt) );
					if ( n < 0 ) 
						{
						perror( "escritura en socket" );
						exit( 1 );
						}	
				}

				/*Espero por comando*/
				memset( buffer, 0, TAM );
				n = read( newsockfd, buffer, TAM-1 );
				if ( n < 0 ) 
					{
					perror( "lectura de socket" );
					exit(1);
					}
				printf( "Recibí el siguiente comando: %s\n", buffer );

				/*Intento de ejecucion de comandos*/
				if(enviar==0)
				{
					if(strncmp(buffer,"update firmware.bin",19)==0 || strncmp(buffer,"start scanning",14)==0)
					 {
						printf("Envio actualizacion de firmware\n");
						enviar=1;
						getcwd(file_to_send,100);		/*Obtengo el path de descarga*/
						strtok(buffer, " ");			/*Descarto el primer eslabon*/
						token=strtok(NULL," ");			/*Obtengo el nombre del archivo a descargar*/
						sprintf(file_to_send, "%s/%s", current_directory, token);
						
					 }
					else if (strncmp(buffer,"obtener telemetria",18)==0) 
						{
						printf("Enviando telemetria\n");
						n = write( newsockfd, "TELEMETRIA", 11 );
						if ( n < 0 ) 
							{
							perror( "escritura en socket" );
							exit( 1 );
							}
						}
					else if(strncmp(buffer,"fin",3)==0)
					{
						printf( "PROCESO %d. Como recibí 'fin', termino la ejecución.\n\n", getpid() );
						exit(0);
					}
					else
					{
						printf("Comando desconocido\n");
						n = write( newsockfd, buffer, strlen(buffer));
						if ( n < 0 ) 
							{
							perror( "escritura en socket" );
							exit( 1 );
							}
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
