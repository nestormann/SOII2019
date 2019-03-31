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
	int correct_input = 0;
	int correct_user = 0; 	
	int correct_pass = 0; 	 										
	char *token;												 
	char *usuario;
	char input[TAM];
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

		while(1) 
		 {

			/*Mensaje de sincronizacion*/
			write( sockfd, "SINC", 5);
            if ( n < 0 ) 
             {
                perror( "escritura de socket" );
                exit( 1 );
             }

            /*Recibo e imprimo el prompt*/ 
			memset( input, '\0', TAM );
			n = read( sockfd, input, TAM );
            if ( n < 0 ) 
             {
                perror( "lectura de socket" );
                exit( 1 );
             }
			printf( "%s", input );

			/*Escritura del comando a ejecutar*/
			memset( input, '\0', TAM );
			fgets( input, TAM-1, stdin );
			write( sockfd, input, strlen(input) );
            if ( n < 0 ) 
             {
                perror( "escritura de socket" );
                exit( 1 );
             }

            /*Verifico si el usuario ingreso 'fin'*/
            input[strlen(input)-1] = '\0';
            if( !strcmp( "fin", input ) ) 
             {
                terminar = 1;
             }

    		memset( input, '\0', TAM );
			n = read( sockfd, input, TAM );
            if ( n < 0 ) 
             {
                perror( "lectura de socket" );
                exit( 1 );
             }
            printf( "%s\n", input );

            if( terminar ) 
             {
     	       	printf( "Finalizando ejecución\n" );
            	exit(0);
             }
		 }
	 }
	return 0;
 } 