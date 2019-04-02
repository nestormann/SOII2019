#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define TAM 4096

int main( int argc, char *argv[] ) 
 {
	int sockfd, puerto, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	int terminar = 0;
	int descargar=0;
	int correct_input = 0; 										
	char *token;												 
	char *usuario;
	char input[TAM];
	int correct_user=0;
	int correct_pass=0;
	int file_size;
    FILE *received_file;
    int remain_data = 0;
	int restart_flag=0;
    ssize_t len;
    char file_path[100]="";
    char current_dir[100]="";
    getcwd(current_dir,100);
	FILE *firmware;
    printf("argc= %d\nargv= %s\n",argc,*argv);
    char firmware_path[100]="./firmware.bin";
    char buff[255];
    firmware = fopen(firmware_path, "r");
    if (firmware == NULL)
     {
        fprintf(stderr, "Error opening file --> %s", strerror(errno));
     }
    else
     {
        fgets(buff, 255, (FILE*)firmware);
        printf("Version del software: %s\n",buff); 
        fclose(firmware);
     }

    /*Se comprueba la sintaxis del comando ingresado, 
      de ser correcto prosigo, de lo contrario finaliza*/
	while(correct_input==0)
	 {

		printf("LOGIN: "); 								
		fgets(input,75,stdin);
		//scanf("%s", input);							
		input[strlen(input)-1]='\0';						
		char delimit[3];									
		delimit[0]=' ';
		delimit[1]='@';
		delimit[2]=':';
		token= strtok(input, delimit);							

		while(token==NULL || strncmp(token,"connect",7))
		 {	
			printf("Comando incorrecto, reingrese\n");
			printf("LOGIN: "); 								
			fgets(input,75,stdin);						
			input[strlen(input)-1]='\0';
			token= strtok(input, delimit);	
		 }

		token= strtok(NULL, delimit);
		if(token==NULL)
		 {
			printf("No ingreso el usuario\n");
			exit(0);
		 }
		else
		 {
			usuario=token;
			token= strtok(NULL, delimit);					
		 }

		int index_argv=1;											
		while(token != NULL)
		{								
			argv[index_argv]=token;
			index_argv++;
			argc=index_argv;
			token= strtok(NULL, delimit);
		 }
		argv[index_argv]='\0';							

		printf("Usuario ingresado: %s\n",usuario );
		correct_input=1;
	 }

	/*Conexion con el puerto del servidor*/
	if ( argc < 3 ) 
	 {
		fprintf( stderr, "Uso %s host puerto\n", argv[0]);
		exit( 0 );
	 }

	puerto = atoi( argv[2] );
	sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	if ( sockfd < 0 ) 
	 {
		perror( "ERROR apertura de socket" );
		exit( 1 );
	 }

	server = gethostbyname( argv[1] );
	if (server == NULL) 
	 {
		fprintf( stderr,"Error, no existe el host\n" );
		exit( 0 );
	 }

	memset( (char *) &serv_addr, '0', sizeof(serv_addr) );
	serv_addr.sin_family = AF_INET;
	bcopy( (char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length );
	serv_addr.sin_port = htons( puerto );
	if ( connect( sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr ) ) < 0 ) 
	 {
		perror( "conexion" );
		exit( 1 );
	 }

	while(1) 
	 {	/*Espera autenticacion del usuario*/
		while(correct_user==0)
		 {
			n = write( sockfd, usuario, strlen(usuario) );				
			if ( n < 0 ) 
			 {
				perror( "escritura de socket" );
				exit( 1 );
			 }
			/*Respuesta de comprobacion*/
			memset( input, '\0', TAM );									
			n = read( sockfd, input, TAM );
			if ( n < 0 ) 
			 {
				perror( "lectura de socket" );
				exit( 1 );
			 }
			if(strncmp(input,"wrongUser",9)==0)
			 {						
				printf( "Usuario desconocido\n" );
				exit(0);
			 }
			/*Usuario correcto, envio la contraseña*/
			if(strncmp(input,"correctUser",11)==0)
			 {	
				printf( "Usuario correcto, ingrese la contraseña: " );
				memset( input, '\0', TAM );
				fgets( input, TAM-1, stdin );
				n = write( sockfd, input, strlen(input) );
				if ( n < 0 ) 
				 {
					perror( "escritura de socket" );
					exit( 1 );
				 }
				correct_user=1;
			 }
		 }
		/*Espera de autenticacion de la contraseña*/
		while(correct_pass==0)
		 {	/*Respuesta de comprobacion*/
			memset( input, '\0', TAM );									
			n = read( sockfd, input, TAM );
			if ( n < 0 ) 
			 {
				perror( "lectura de socket" );
				exit( 1 );
			 }
			if(strncmp(input,"wrongPass",9)==0)
			 {	
				printf( "Contraseña incorrecta, reingrese: " );
				memset( input, '\0', TAM );
				fgets( input, TAM-1, stdin );
				n = write( sockfd, input, strlen(input) );
				if ( n < 0 ) 
				 {
					perror( "escritura de socket" );
					exit( 1 );
				 }
			 }
			if(strncmp(input,"correctPass",11)==0)
			 {	
				printf( "Contraseña correcta, conexion establecida\n" );
				correct_pass=1;
			 }
		 }
		/*Procedimiento de descarga*/
		if(descargar==1)
		 {
			/* Receiving file size */
			memset( input, '\0', TAM );
	        recv(sockfd, input, TAM, 0);

			if(strncmp(input,"nofile",6)==0)
			 {
			 	printf("NO EXISTE EL ARCHIVO SOLICITADO\n");
			 	descargar=0;
			 }
			else
			 {
			 	printf("DESCARGANDO\n");
		        file_size = atoi(input);
		        fprintf(stdout, "File size : %d\n", file_size);

		        received_file = fopen(file_path, "w");
		        if (received_file == NULL)
		         {
		            fprintf(stderr, "Failed to open file foo --> %s\n", strerror(errno));
		            exit(EXIT_FAILURE);
		         }

		        remain_data = file_size;

				clock_t start = clock();

		        memset( input, '\0', TAM );					
		        while (((len = recv(sockfd, input, TAM, 0)) > 0) && (remain_data >=0))
		         {
		            fwrite(input, sizeof(char), len, received_file);
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
		
		if(descargar==0)
		 {
			/*Mensaje de sincronizacion*/
			write( sockfd, "SINC", 5);
			if ( n < 0 ) 
			{
				perror( "escritura de socket" );
				exit( 1 );
			}
	   	 }

		/*Recepcion e impresion del prompt*/
		memset( input, '\0', TAM );					
		n = read( sockfd, input, TAM );
		if ( n < 0 ) 
		 {
			perror( "lectura de socket" );
			exit( 1 );
		 }
		printf("%s",input);	

		/*Ingreso y envio del comando a ejecutar*/
		memset( input, '\0', TAM );
		fgets( input, TAM-1, stdin );
		n = write( sockfd, input, strlen(input) );
		if ( n < 0 ) 
		 {
			perror( "escritura de socket" );
			exit( 1 );
		 }

		/*Verificacion de terminacion del programa*/
		input[strlen(input)-1] = '\0';
		if( !strcmp( "fin", input ) ) 
		 {
			terminar = 1;
		 }
		if( terminar ) 
		 {
			printf( "Finalizando ejecución\n" );
			exit(0);
		 }

		/*Verificacion de intento de descargar*/
		if(strncmp(input,"update firmware.bin",19)==0 || strncmp(input,"start scanning",14)==0)  
		{
			if(strncmp(input,"update firmware.bin",19)==0) 
			{
				printf("Me voy a cagar reiniciando\n");
				restart_flag=1;
			}
			descargar = 1;
			strtok(input, " ");	
			token=strtok(NULL," ");		/*Nombre del archivo a descargar*/
			sprintf(file_path, "%s/%s", current_dir, token);
			
		}
		else
		{	/*Recepcion de salida del servidor*/
			memset( input, '\0', TAM );					
			n = read( sockfd, input, TAM );
			if ( n < 0 ) 
			{
				perror( "lectura de socket" );
				exit( 1 );
			}
			printf("%s\n",input);	
		}
	 }
	return 0;
 } 
