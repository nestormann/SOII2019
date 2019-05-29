//	Nestor Javier Ortmann 39024999
//	Andrés Salvatierra 	  39611008
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/wait.h>

#define COLOR_GREEN "\033[0;32m"
#define COLOR_BLUE  "\033[0;34m"
#define	COLOR_WHITE "\033[0;0m"

#define write 1
#define read 0


void path(char **argv);
void busqueda(char **argv, char *input);
int main(int argc, char** argv)
{	
	// char* readInput = strchr(getenv("QUERY_STRING"),'=')+1;

	// char *inputParse = strdup(readInput);

	// char anio[10]={0};
	// char dia[10]={0};

	// strcat(anio,inputParse);
	// strcat(dia,inputParse);

	// char *year=strtok(dia,"&");
	// char *day=strtok(NULL," ");

	// day=strtok(day,"=");
	// day=strtok(NULL," ");

	char *year="2017";
	char *day="300";

	char *command="aws s3 ls --recursive noaa-goes16/ABI-L2-CMIPF/";
	char *flags="--no-sign-request | grep M3C13";

	char input[255];		//Entrada de usuario
	sprintf(input,"%s%s/%s/%s %s%c",command,year,day,"17",flags,'\0');
	printf("Input: %s\n",input);
	char *argvAux[10];		//Argv del primer comando
	char *argvAux2[10];		//Argv del segundo comando (si es que lo hubiese)
	char *token;			//Variable que toma elabones individuales del string 
	char *home=getenv("HOME"); //Variable que almacena el directorio home
	//char *user= getenv("USER");	//Variable que almacena el usuario 
	char *host= (char*) malloc(1024); //Reservamos espacio para el hostname
	gethostname(host, 1024);
	//char dirActual[100]; //Almacenamos el directorio actual 
					
	//printf(COLOR_GREEN"%s@%s%s:%s~%s%s$ ",user,host,COLOR_WHITE,COLOR_BLUE,getcwd(dirActual,100),COLOR_WHITE);

	//fgets(input,75,argv[1]);	//Toma el ingreso del teclado y lo guarda en input
	
	// input[strlen(input)-1]='\0';	//Para eliminar el caracter agregado por el ENTER al ingresar por teclado
	if(strlen(input)!=0)				
	{

		char *waiter= strrchr(input,'&');	//Se utiliza para comprobar la existencia del &
		char *barra= strrchr(input,'|'); 	//Se utiliza para comprobar la existencia del |
		char *mayor= strrchr(input, '>');	//Se utiliza para comprobar la existencia del >
		char *menor= strrchr(input, '<');	//Se utiliza para comprobar la existencia del <

		if(waiter!=NULL) input[strlen(input)-2]='\0';	//Borra '&' del arreglo de entrada siempre y cuando aparezca

		token= strtok(input, " ");	//Agrego en token el primer eslabon de la entrada

		int indexArg=0;		//Indice para recorrer el arreglo

		while(token != NULL)	//Mientras siga habiendo eslabones los voy agregando en el argv
			{	

				if(token[0]=='|' || token[0]=='<' || token[0]=='>') break;
				argvAux[indexArg]=token;
				indexArg++;
				argc=indexArg;
				token= strtok(NULL, " ");
			}
		argvAux[indexArg]='\0';	//Se utiliza para poder cortar la lectura del arreglo

		if(!strncmp(argvAux[0],"cd",2))	//Hace change directory con cd
		{
			if(argvAux[1]!=NULL)
			chdir(argvAux[1]);
			else
			chdir(home);
		}

		else
		{
			if(barra!=NULL || mayor!=NULL || menor!=NULL){
				token= strtok(NULL, " ");	//elimino el caracter(sea |,< o >) y almaceno en el argv del segundo comando o archivo
				indexArg=0;			
				while(token != NULL)
				{	
					argvAux2[indexArg]=token;
					indexArg++;
					argc=indexArg;
					token= strtok(NULL, " ");
				}
				argvAux2[indexArg]='\0';
			}

			pid_t hijoPid;	//pid para el hijo

			if((hijoPid = fork())<0)
			{
				perror("fork hijo");
				exit(1);
			}

			else if(hijoPid==0){	//Codigo para el hijo

				if(barra==NULL){
					if(mayor!=NULL) freopen(argvAux2[0],"w", stdout); //Para copiar la salida de un comando a un archivo
					if(menor!=NULL)	freopen(argvAux2[0],"r",stdin); //Para hacer que un archivo sea entrada de un comando
					busqueda(argvAux,input);
				}	
				
				else{
					pid_t nietoPid;	//pid para el nieto
					int p[2];
					pipe(p);
					if((nietoPid = fork())<0)
					{
						perror("fork nieto");
						exit(1);
					}
						
					if(nietoPid==0){		//Codigo para el nieto
						close(p[read]);
						dup2(p[write],1);	//salida del nieto a entrada del hijo
						busqueda(argvAux,input); 
					}
					else{
						close(p[write]);
						dup2(p[read],0);	
						wait(&nietoPid);	//Hijo espera la ejecucion del nieto
						busqueda(argvAux2,input);	
					}
				}
			}

			else
			{
				if(waiter==NULL || strlen(waiter)==0)
				wait(&hijoPid);			//Padre espera la ejecucion del hijo
			}			
		}
	}
}
	


void busqueda(char **argv, char *input)
{
	if(strncmp(argv[0],"/",1)==0) //BUSQUEDA DE PATH ABSOLUTO
			{
				execv(input,argv);
			}

			else if(strncmp(argv[0],"~",1)==0)	//BUSQUEDA RELATIVA
			{
				char *home=getenv("HOME");
				strcat(home,strchr(argv[0],'/'));	//Tratamiento del string para lograr la sintaxis correcta
				printf("%s\n",home );
				execv(home,argv);
			}

			else if(strncmp(argv[0],"..",2)==0)	//BUSQUEDA PATH ANTERIOR
			{
				chdir("..");		//Para cambiar al directorio anterior			
				char dirActual[100];
				char *oldpwd= getcwd(dirActual,100); //El directorio anterior se guarda en el actual
				printf("%s\n",oldpwd );
				strcat(oldpwd,strchr(argv[0],'/')); //Tratamiento del string para lograr la sintaxis correcta
				printf("%s\n",oldpwd );
				execv(oldpwd,argv);
			}

			else if(strncmp(argv[0],".",1)==0)	//BUSQUEDA PATH ACTUAL
			{
				char dirActual[100];
				char *pwd = getcwd(dirActual,100);
				strcat(pwd,strchr(argv[0],'/'));
				printf("%s\n",pwd );
				execv(pwd,argv);
			}

			else
			{
				path(argv); //BUSQUEDA PATH COMPLETO
			}
}

void path(char** argv)
{
		int indexPath=0,cantRutas=1;
		char *path = getenv("PATH");
		char *token;
		

		for(int k=0; path[k]!='\0'; k++)		//Calculo la cantidad de PATH
		{
				if(path[k]==':')
				cantRutas++;
		} 								

		char *rutas[cantRutas];		//Arreglo que contiene todos los path por separado
	
		token= strtok(path, ":");			
		
		while(token != NULL)
		{	
				rutas[indexPath]=token;			
				indexPath++;
				token= strtok(NULL, ":");
		}

			indexPath=0;

			while(indexPath<cantRutas)						//Se prueba en cada path si se encuentra el archivo y lo ejecuta
			{
					char dirActual[100]="";
					strcat(dirActual,rutas[indexPath]);		
					strcat(dirActual,"/");
					strcat(dirActual,argv[0]);
					execv(dirActual,argv);
					indexPath++;
			}		
			if(execv(rutas[indexPath],argv)<0)			//En caso que no se haya encontrado en ninguna ruta, tira error
			{
				perror("exec");
				exit(1);
			} 
}