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
#include <sys/resource.h>
#include <sys/un.h>

#define TAM 4096
#define SOCK_PATH  "../servidor/prueba"
#define SOCK_PATHDAT  "../servidor/pruebadat"

int main( int argc, char *argv[] ) 
 {
	time_t start_time = time(NULL); 	
	int sockfd, n,servlen;
	int sock_udp;
	socklen_t tamano_direccion;
	struct sockaddr_un serv_addr;
	struct sockaddr_un dest_addr;
	int descargar=0;						
	char *token;			
	char buffer[TAM];
	//char input[TAM];
	int enviar=0;
	//int sinc=0;
	int file_size;
	char file_sizee[256];
    FILE *received_file;
    int remain_data = 0;
	int restart_flag=0;
    char file_path[100]="";
    char current_dir[100]="";
    getcwd(current_dir,100);
	char file_to_send[100]="";
	FILE *firmware;
    char firmware_path[100]="./firmware.bin";
	char current_directory[100]; 				/*Almacenamos el directorio actual*/
	/*Datos del satelite*/
    char ver_sat[255];
	char id_sat[5]="R2D2";
	char *host= (char*) malloc(1024); 
	int fd;
	struct stat file_stat;
	//int sinc;
	int sent_bytes = 0;
	off_t offset;
	ssize_t len;
	gethostname(host, 1024);
	char prompt[TAM];
	sprintf(prompt, "%s@%s:~%s$ ",id_sat,host,getcwd(current_directory,100));

    firmware = fopen(firmware_path, "r");
    if (firmware == NULL)
     {
        fprintf(stderr, "Error opening file --> %s", strerror(errno));
     }
    else
     {
        fgets(ver_sat, 255, (FILE*)firmware);
        printf("Version del software: %s\n",ver_sat);
		printf("ID: %s\n", id_sat); 
        fclose(firmware);
	 }

	memset( (char *)&serv_addr, '\0', sizeof(serv_addr) );
	serv_addr.sun_family = AF_UNIX;
	strcpy( serv_addr.sun_path, SOCK_PATH);//argv[1] );
	servlen = strlen( serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if ( (sockfd = socket(AF_UNIX, SOCK_STREAM, 0) ) < 0) 
     {
		perror( "creación de socket" );
		exit( 1 );
	 }

 	if ( connect( sockfd, (struct sockaddr *)&serv_addr, servlen ) < 0 ) 
     {
		perror( "conexión" );
		exit( 1 );
	 }

	/* Creacion de socket */
	if(( sock_udp = socket(AF_UNIX, SOCK_DGRAM, 0) ) < 0 ) 
		perror("socket" );

	/* Inicialización y establecimiento de la estructura del cliente */
	memset( &dest_addr, 0, sizeof( dest_addr ) );
	dest_addr.sun_family = AF_UNIX;
	strncpy( dest_addr.sun_path, SOCK_PATHDAT, sizeof( dest_addr.sun_path ) );
	while(1) 
	 {	
		/*while(!sinc) 
		{
				
			sinc=1;
		}*/
		/*Procedimiento de envio*/
				if(enviar==1 && descargar==0)
				 {
					fd = open(file_to_send, O_RDONLY);
        			if (fd == -1)
			         {
			            fprintf(stderr, "Error opening file --> %s", strerror(errno));
			            n = write( sockfd,"nofile" , 6);		
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
				        len = send(sockfd, file_sizee, sizeof(file_sizee), 0);

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
						n = read( sockfd, buffer, TAM-1 );
						if ( n < 0 ) 
						{
							perror( "lectura de socket" );
							exit(1);
						}
						

				        /* Sending file data */
				        while (((sent_bytes = sendfile(sockfd, fd, &offset, TAM)) > 0) && (remain_data >= 0) )
				         {
				            remain_data -= sent_bytes;
				         }

				        printf("FINALIZO DESCARGA\n");
				        /*Espera confirmacion para finalizar la descarga*/
				        memset( buffer, 0, TAM );
						n = read( sockfd, buffer, TAM-1 );
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
					recv(sockfd, buffer, TAM, 0);

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
						n = write( sockfd, "sinc", 5);
						if ( n < 0 ) 
						{
							perror( "escritura en socket" );
							exit( 1 );
						}					
						while (((len = recv(sockfd, buffer, TAM, 0)) > 0) && (remain_data >=0))
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
						n = write( sockfd, "done", 5);
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
		/*Envio del prompt a mostar en el cliente*/
		if(descargar==0 && enviar==0)
		{	
			/*Recepcion de sinc*/
			memset( buffer, '\0', TAM );					
			n = read( sockfd, buffer, TAM );
			if ( n < 0 ) 
			{
				perror( "lectura de socket" );
				exit( 1 );
			}
			printf("%s\n",prompt);
			n = write( sockfd,prompt , strlen(prompt));	
			if ( n < 0 ) 
			{
				perror( "escritura en socket" );
				exit( 1 );
			}
		}

		/*Espera de ingreso de comandos*/
		memset( buffer, 0, TAM );								
		n = read( sockfd, buffer, TAM-1 );
		if ( n < 0 ) 
			{
			perror( "lectura de socket" );
			exit(1);
			}
		buffer[strlen(buffer)-1] = '\0';
		printf( "Recibí el siguiente comando: %s\n", buffer );

		/*Intento de ejecucion de comandos*/
		if(descargar==0 && enviar==0)
		{
			if(strncmp(buffer,"start scanning",14)==0)
			{
				printf("Envio scanning\n");
				enviar=1;
				getcwd(file_to_send,100);		/*Obtengo el path de descarga*/
				strtok(buffer, " ");			/*Descarto el primer eslabon*/
				token=strtok(NULL," ");			/*Obtengo el nombre del archivo a descargar*/
				sprintf(file_to_send, "%s/%s", current_directory, token);
			}
			else if(strncmp(buffer,"update firmware.bin",19)==0) 
			{
				restart_flag=1;
				descargar = 1;
				strtok(buffer, " ");	
				token=strtok(NULL," ");		/*Nombre del archivo a descargar*/
				sprintf(file_path, "%s/%s", current_dir, token);
			}
			else if (strncmp(buffer,"obtener telemetria",18)==0) 
			{
				printf("Enviando telemetria\n");
				/*UPTIME*/
				time_t current_time = time(NULL); 
				int uptime = (int)(current_time - start_time);
				/*VSIZE*/
				char vsz[64];
				FILE *ps;
				char command[128]="";
				pid_t pid = getpid();
				sprintf(command, "ps -Ao vsize,pid | grep %d", pid);
				ps = popen(command,"r");
				if (ps == NULL)
				{
					fprintf(stderr, "Error opening file --> %s", strerror(errno));
				}
				else
				{
					fgets(vsz, 255, (FILE*)ps);
					token=strtok(vsz, " ");
					fclose(ps);
				}
				/*CPU usage*/
				struct rusage r_usage;
				getrusage(RUSAGE_SELF, &r_usage);
				int sCPUusage= r_usage.ru_utime.tv_sec;
				int usCPUusage= r_usage.ru_utime.tv_usec;
				char telemetria[4096]="";
				tamano_direccion = sizeof( dest_addr );
				sprintf(telemetria, "Version firmare: %s\nID: %s\nUptime[s]: %d\nCPU usage: %ds %dus\nVSIZE: %s\n", 
									ver_sat,id_sat, uptime,sCPUusage,usCPUusage,vsz);
				n = sendto( sock_udp, (void *)telemetria, TAM, 0, (struct sockaddr *)&dest_addr, tamano_direccion );
				if ( n < 0 ) 
				{
					perror( "Escritura en socket" );
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
			}
		}	
	 }
	return 0;
 } 
