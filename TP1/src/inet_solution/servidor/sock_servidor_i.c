#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <sys/wait.h>
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
	int user_verification=1;
	int pass_verification=1;
	char file_path[100]="";
    char current_dir[100]="";
    getcwd(current_dir,100);
	int intentos=0;
	char current_directory[100]; 				/*Almacenamos el directorio actual*/
	char *host= (char*) malloc(1024); 
	gethostname(host, 1024);
	char prompt[TAM];
	sprintf(prompt, "%s@%s:~%s$ ",usuario,host,getcwd(current_directory,100));
	int enviar=0;
	int descargar=0;
	int fd;
	struct stat file_stat;
	//int sinc;
	int sent_bytes = 0;
	off_t offset;
	ssize_t len;
	char file_sizee[256];
	int remain_data;
	char file_to_send[100]="";
	int restart_flag=0;
    char *token;
	int file_size;
	FILE *received_file;
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
	 {	
		/*
			Autenticacion
		*/
		while(pass_verification!=0 || user_verification!=0)
		{	
			if(intentos==3)
			{
				printf("FALLO EN LA AUTENTICACION, FIN DEL PROGRAMA\n");
				exit(1);
			}
			else if (intentos>0) 
			{
				printf("Usuario y/o pass incorrecto, reintento %d de 2\n",intentos);
			}
		
			printf("INGRESE EL USUARIO: ");
			fgets(buffer,75,stdin);
			user_verification=strncmp(buffer,usuario,strlen(buffer)-1);
			printf("INGRESE LA CONTRASEÑA: ");
			fgets(buffer,75,stdin);
			pass_verification=strncmp(buffer,password,strlen(buffer)-1);
			intentos++;
		} 
		printf("Espera de conexion con el satelite\n");
		int newsockfd, pid;
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
			 {	
				/*Procedimiento de envio*/
				if(enviar==1 && descargar==0)
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

				        //clilen = sizeof(struct sockaddr_in);

				         /* Sending file size */
				        sprintf(file_sizee, "%zd", file_stat.st_size);
				        len = send(newsockfd, file_sizee, sizeof(file_sizee), 0);

				        if (len < 0)
				         {
				              fprintf(stderr, "Error on sending greetings --> %s", strerror(errno));
				              exit(EXIT_FAILURE);
				         }

				        fprintf(stdout, "Server sent %zd bytes for the size\n", len);

				        offset = 0;
				        remain_data = file_stat.st_size;

						/*Recibo el sinc*/
						memset( buffer, 0, TAM );
						n = read( newsockfd, buffer, TAM-1 );
						if ( n < 0 ) 
						{
							perror( "lectura de socket" );
							exit(1);
						}

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
				
				/*Procedimiento de descarga*/
				if(descargar==1 && enviar==0)
				{
					/* Receiving file size */
					memset( buffer, '\0', TAM );
					recv(newsockfd, buffer, TAM, 0);

					if(strncmp(buffer,"nofile",6)==0)
					{
						printf("NO EXISTE EL ARCHIVO SOLICITADO\n");
						descargar=0;
					}
					else
					{
						printf("DESCARGANDO\n");
						file_size = atoi(buffer);
						fprintf(stdout, "File size : %d\n", file_size);

						received_file = fopen(file_path, "w");
						if (received_file == NULL)
						{
							fprintf(stderr, "Failed to open file foo --> %s\n", strerror(errno));
							exit(EXIT_FAILURE);
						}

						remain_data = file_size;

						clock_t start = clock();

						memset( buffer, '\0', TAM );					
						while (((len = recv(newsockfd, buffer, TAM, 0)) > 0) && (remain_data >=0))
						{
							fwrite(buffer, sizeof(char), len, received_file);
							remain_data -= len;
							if(remain_data==0) 
								break;
						}

						clock_t end = clock();
						float seconds = (float)(end - start) / CLOCKS_PER_SEC;
						printf("TERMINO DESCARGA EN %f segundos\n",seconds);
						fclose(received_file);
						/*Envio confirmacion de fin de descarga*/
						n = write( newsockfd, "done", 5);
						if ( n < 0 ) 
						{
							perror( "escritura de socket" );
							exit( 1 );
						}
						if(restart_flag)
						{
							execvp(argv[0],argv);
						}
						descargar=0;
					}
				}
				

				if(enviar==0 && descargar==0)
				{
					/*Recibo el prompt*/
					n = write( newsockfd, "SINC", 4 );
					if ( n < 0 ) 
					 {
						perror( "escritura en socket" );
						exit( 1 );
				   	 }
					/*Recibo el prompt*/
					memset( buffer, 0, TAM );
					n = read( newsockfd, buffer, TAM-1 );
					if ( n < 0 ) 
					 {
						perror( "lectura de socket" );
						exit(1);
				 	 }
					printf("%s",buffer);
				}

				/*Ingreso y envio del comando a ejecutar*/
				memset( buffer, '\0', TAM );
				fgets( buffer, TAM-1, stdin );
				n = write( newsockfd, buffer, strlen(buffer) );
				if ( n < 0 ) 
				{
					perror( "escritura de socket" );
					exit( 1 );
				}

				/*Verificacion de terminacion del programa*/
				buffer[strlen(buffer)-1] = '\0';
				if( !strcmp( "fin", buffer ) ) 
				{
					printf( "Finalizando ejecución\n" );
					exit(0);
				}
				/*Verificacion de intento de descargar*/
				if(strncmp(buffer,"start scanning",14)==0)  
				{
					descargar = 1;
					strtok(buffer, " ");	
					token=strtok(NULL," ");		/*Nombre del archivo a descargar*/
					sprintf(file_path, "%s/%s", current_dir, token);
				}
				else if(strncmp(buffer,"update firmware.bin",19)==0)  
				{
					printf("ENVIO FIRMWARE\n");
					enviar = 1;
					getcwd(file_to_send,100);		/*Obtengo el path de descarga*/
					strtok(buffer, " ");			/*Descarto el primer eslabon*/
					token=strtok(NULL," ");			/*Obtengo el nombre del archivo a descargar*/
					sprintf(file_to_send, "%s/%s", current_directory, token);
					restart_flag=1;
				}
				else if (strncmp(buffer,"obtener telemetria",18)==0) 
				{
					memset( buffer, 0, TAM );
					n = recvfrom( sock_udp, buffer, TAM-1, 0, (struct sockaddr *)&serv_addr_udp, &tamano_direccion );
					if ( n < 0 ) {
						perror( "lectura de socket" );
						exit( 1 );
					}
					printf( "Version: %s\n", buffer );
				}
				
				else
				{	
					/*Recepcion de salida del servidor*/
					memset( buffer, '\0', TAM );					
					n = read( newsockfd, buffer, TAM );
					if ( n < 0 ) 
					{
						perror( "lectura de socket" );
						exit( 1 );
					}
					printf("%s\n",buffer);	
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
