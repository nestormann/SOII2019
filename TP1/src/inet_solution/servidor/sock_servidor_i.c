#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
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

int main( int argc, char *argv[] ) 
 {
	int sockfd, puerto;//, stdout_cpy;
	int sock_udp, puerto_udp;
	socklen_t tamano_direccion;
	socklen_t clilen;
	char buffer[TAM];
	struct sockaddr_in serv_addr, cli_addr, serv_addr_udp;
	int n;
	char usuario[]="nestor";
	char password[]="parapam";
	int user_verification=0;
	int pass_verification=0;
	char current_directory[100]; 				/*Almacenamos el directorio actual*/
	char *host= (char*) malloc(1024); 
	gethostname(host, 1024);
	char prompt[TAM];
	sprintf(prompt, "%s@%s:~%s$ ",usuario,host,getcwd(current_directory,100));
	int enviar=0;
	int fd;
	struct stat file_stat;
	int sent_bytes = 0;
	off_t offset;
	ssize_t len;
	char file_size[256];
	int remain_data;
	char file_to_send[100]="";
	int restart_flag=0;
    char *token;
    /*char *argv_to_baash[10];*/
	if ( argc < 2 ) 
	 {
        fprintf( stderr, "Uso: %s <puerto>\n", argv[0] );
		exit( 1 );
	 }

	/*Inicializacion socket TCP*/
	sockfd = socket( AF_INET, SOCK_STREAM, 0);
	if ( sockfd < 0 ) 
	 { 
		perror( " apertura de socket ");
		exit( 1 );
	 }
	memset( (char *) &serv_addr, 0, sizeof(serv_addr) );
	puerto = atoi( argv[1] );
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons( puerto );

	if( bind( sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr) ) < 0 ) {
		perror( "ERROR en binding" );
		exit( 1 );
	}

	printf( "Proceso: %d - socket disponible: %d\n", getpid(), ntohs(serv_addr.sin_port) );

	listen( sockfd, 5 );
	clilen = sizeof( cli_addr );

	/*Inicializacion socket UDP*/
	sock_udp = socket( AF_INET, SOCK_DGRAM, 0 );
	if (sock_udp < 0) 
	 { 
		perror("ERROR en apertura de socket");
		exit( 1 );
	 }

	memset( &serv_addr_udp, 0, sizeof(serv_addr_udp) );
	puerto_udp = puerto+1;
	serv_addr_udp.sin_family = AF_INET;
	serv_addr_udp.sin_addr.s_addr = INADDR_ANY;
	serv_addr_udp.sin_port = htons( puerto_udp );
	memset( &(serv_addr_udp.sin_zero), '\0', 8 );

	if ( bind(sock_udp, ( struct sockaddr *) &serv_addr_udp, sizeof( serv_addr_udp ) ) < 0 ) 
	 {
		perror( "ligadura" );
		exit( 1 );
	 }
	
	printf( "Socket disponible: %d\n", ntohs(serv_addr_udp.sin_port) );

	tamano_direccion = sizeof( struct sockaddr );

	while( 1 ) 
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

			while ( 1 ) 
			 {	/*Verificacion del usuario*/
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
						printf("Contraseña correcta, conexion establecida\n");
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
						{
					    	enviar=0;
							if(restart_flag)
							{
								printf("fin de conexion\n");
								exit(0);
							} 
						}

						
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

				/*Envio del prompt a mostar en el cliente*/
				if(enviar==0)
				 {
					n = write( newsockfd,prompt , strlen(prompt));	
					if ( n < 0 ) 
					{
						perror( "escritura en socket" );
						exit( 1 );
					}
				 }

				/*Espera de ingreso de comandos*/
				memset( buffer, 0, TAM );								
				n = read( newsockfd, buffer, TAM-1 );
				if ( n < 0 ) 
				 {
					perror( "lectura de socket" );
					exit(1);
				 }
				buffer[strlen(buffer)-1] = '\0';
				printf( "Recibí el siguiente comando: %s\n", buffer );

				/*Intento de ejecucion de comandos*/
				if(enviar==0)
				 {
					if(strncmp(buffer,"update firmware.bin",19)==0 || strncmp(buffer,"start scanning",14)==0)
					 {
						if(strncmp(buffer,"update firmware.bin",19)==0) 
						{
							printf("Actualizacion del satelite\n");
							restart_flag=1;
						} 
						printf("Envio actualizacion de firmware\n");
						enviar=1;
						getcwd(file_to_send,100);		/*Obtengo el path de descarga*/
						strtok(buffer, " ");			/*Descarto el primer eslabon*/
						token=strtok(NULL," ");			/*Obtengo el nombre del archivo a descargar*/
						sprintf(file_to_send, "%s/%s", current_directory, token);
						
					 }
					else if (strncmp(buffer,"obtener telemetria",18)==0) 
					 {
						printf("Esperando telemetria:\n");
						n = write( newsockfd, "TELEMETRIA", 11 );
						if ( n < 0 ) 
						 {
							perror( "escritura en socket" );
							exit( 1 );
						 }
						memset( buffer, 0, TAM );
						n = recvfrom( sock_udp, buffer, TAM-1, 0, (struct sockaddr *)&serv_addr_udp, &tamano_direccion );
						if ( n < 0 ) {
							perror( "lectura de socket" );
							exit( 1 );
						}
						printf( "Version: %s\n", buffer );
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
