#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>
#include <errno.h>
#include <sys/un.h>

#define TAM 4096

int main( int argc, char *argv[] ) 
 {
	int sockfd, n,servlen;
	struct sockaddr_un serv_addr;
	int terminar = 0;
    int scanning = 0;
	int correct_input = 0; 										
	char *token;												 
	char *usuario;
	char input[TAM];
	int correct_user=0;
	int correct_pass=0;
    int file_size;
    FILE *received_file;
    int remain_data = 0;
    ssize_t len;
    char file_path[100]="";
    char current_dir[100]="";
    getcwd(current_dir,100);

    /*Se comprueba la sintaxis del comando ingresado, 
      de ser correcto prosigo, de lo contrario finaliza*/
	while(correct_input==0)
	 {

		printf("LOGIN: "); 								
		fgets(input,75,stdin);							
		input[strlen(input)-1]='\0';						
		char delimit[3];									
		delimit[0]=' ';
		delimit[1]='@';
		delimit[2]=':';
		token= strtok(input, delimit);							
/*
		if(token==NULL || strncmp(token,"connect",7))
		 {	
			printf("Comando desconocido\n");
			exit(0);
		 }

		token= strtok(NULL, delimit);*/
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

	/*Conexion con el servidor*/
	if ( argc < 2 ) 
     {
        fprintf( stderr, "Uso %s archivo\n", argv[0]);
        exit( 0 );
     }

	memset( (char *)&serv_addr, '\0', sizeof(serv_addr) );
	serv_addr.sun_family = AF_UNIX;
	strcpy( serv_addr.sun_path, argv[1] );
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
				printf( "Contraseña correcta, conexion establecida con el servidor\n" );
				correct_pass=1;
			 }
		 }
		/*Procedimiento de descarga*/
		if(scanning==1)
		 {
			/* Receiving file size */
			memset( input, '\0', TAM );
	        recv(sockfd, input, TAM, 0);

			if(strncmp(input,"nofile",6)==0)
			 {
			 	printf("NO EXISTE EL ARCHIVO SOLICITADO\n");
			 	scanning=0;
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
		        memset( input, '\0', TAM );					
		        while (((len = recv(sockfd, input, TAM, 0)) > 0) && (remain_data >=0))
		         {
		            fwrite(input, sizeof(char), len, received_file);
		            remain_data -= len;
		            if(remain_data==0) 
		            	break;
		         }
		        printf("TERMINO DESCARGA\n");
		        fclose(received_file);
		        /*Envio confirmacion de fin de descarga*/
			   	n = write( sockfd, "done", 5);
				if ( n < 0 ) 
				 {
					perror( "escritura de socket" );
					exit( 1 );
				 }  
				scanning=0;
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

		/*Verificacion de intento de scanning*/
		if(strncmp(input,"scanning",9)==0) 
		 {
			scanning = 1;
			strtok(input, " ");	
			token=strtok(NULL," ");		/*Nombre del archivo a scanning*/
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
			printf("%s",input);	
		 }
	 }
	return 0;
 } 