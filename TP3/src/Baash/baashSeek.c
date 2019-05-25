#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <string.h>
#include <dirent.h>

#include "baash.h"

#define MAX_PATH_LENGHT_SIZE 1024

/*	Toma la entrada del usuario (comando -argumento1 -argumento2 ...) y la carga en el arreglo
	argv[].
	Si un operador de IPC fue ingresado (comandoA -argumentoA1 | comandoB -argumentoB1) o (comando < archivo) o (comando > archivo)
	se activa una bandera (distinto valor para cada caso) y finaliza el parseo.
*/
void parse(char** readLine, int *argc, char* argv[], int *ipc, int *argindex) {

	char *pointSpace = NULL;                 
	char *inputParse = strdup(*readLine);

	do {
		pointSpace = strchr(inputParse, '+');	/* Corta la string en donde encuentra un espacio, para separar comandos y argumentos. */
		if (pointSpace != NULL) {
			pointSpace[0] = 0;
		}
		if((*inputParse=='|')||(*inputParse=='>')||(*inputParse=='<')) {
			switch(*inputParse){
				case '|':
					*ipc = 1;
					break;
				case '>':
					*ipc = 2;
					break;
				case '<':
					*ipc = 3;
					break;
			}
			argv[*argindex] = 0;		/* En el programa principal se parsea de nuevo, por lo que hay que separar el primer proceso+args de argv[] con un caracter nulo.*/
			*readLine = inputParse+2;	/* Modifica la referencia a la variable original para que, al parsear de nuevo, se lea desde el segundo proceso/archivo.*/
			break;			
		}
		argv[*argindex] = inputParse;	/* Si no hay IPC, simplemente se va cargando la string cortada en argv, siguiendolo con un indice.*/
		*argc = *argindex+1;
		*argindex = *argindex + 1;
		inputParse = pointSpace + 1;	/* Toma la string que estaba despues de la que fue cortada. (despues del espacio encontrado)*/
	} while (pointSpace != NULL);
	argv[*argindex] = 0;				/* Es necesario que siempre el final del arreglo argv[] sea un caracter nulo para identificar el fin de los argumentos.*/
}

/*	Toma un path (absoluto o relativo) escrito por el usuario y lo traduce en un PATH ABSOLUTO.
	*/
const char* searchPath(char *path, char *home, char* userInput){

	char * relativePath = (char*) malloc(MAX_PATH_LENGHT_SIZE);

	switch(*userInput) {
		case '/' :                                          /* Se escribio un path absoluto. (/)*/
			return userInput;
		case '~' :                                          /* Se escribio un path relativo al home (~/)*/
			relativePath = home;		                    
			userInput++;                                    /* Elimina el '~'*/
			return formatAndSearch(relativePath,userInput);      
		case '.' :
			if(userInput[1] == '/') {                       /* Se escribio un path relativo al directorio actual (./)*/
				relativePath = getcwd(relativePath,1024);  
				userInput++;                                /* Elimina el '.'*/
				return formatAndSearch(relativePath,userInput);
			}
			else if(userInput[1] == '.') {                  /* Se escribio un path relativo al directorio anterior al actual (../)*/
				relativePath = getcwd(relativePath,1024);   
				*strrchr(relativePath,'/') = 0;             
				userInput++;                                /* Elimina el '..'*/
				userInput++;                                
				return formatAndSearch(relativePath,userInput);
			}
		default: {                                           /* Se escribio solo un nombre de proceso. Hay que buscar en los directorios de la variable PATH.*/
			char *pointerSeparator = NULL;					
			char *pathSplit = strdup(path);					
			const char *fileFound = NULL;
			do {											 /* en path tenemos todos los directorios separados por :*/
				pointerSeparator = strchr(pathSplit, ':');	 /* va cortando la string para obtener cada directorio por separado y buscar .en cada uno de ellos.*/
				if (pointerSeparator != NULL)
					pointerSeparator[0] = 0;
				fileFound = searchFile(pathSplit,userInput); 
				if(fileFound!=NULL)
					return fileFound;     					 /* Se encontro el proceso en PATH*/
				pathSplit = pointerSeparator + 1;      
			} 
			while (pointerSeparator != NULL);
			return NULL;									 /* No se encontro el proceso en PATH*/
		}
	}

}

const char* formatAndSearch(char* relativePath, char* fileName) {
		char * finalPath = (char *) malloc(MAX_PATH_LENGHT_SIZE);   /* Variable que almacenara el directorio absoluto sin el nombre del archivo*/
		strcpy(finalPath,relativePath);                             /* Copia el path correspondiente al simbolo usado (home si ~, actual si ., etc...)*/
		strcat(finalPath,fileName);                                 /* Concatena el directorio pasado en el shell, que tiene al final el nombre del archivo*/
		*strrchr(finalPath,'/') = 0;                                /* Corta el nombre del archivo para que solo quede el directorio absoluto*/
		fileName = strrchr(fileName,'/')+1;                         /* El directorio que se paso en el shell, lo convierte en SOLO el nombre del archivo, sin la barra / */
		return searchFile(finalPath,fileName);                      /* Llama a la funcion que busca en finalPath (directorio absoluto final), el archivo de nombre fileName.*/
}
/* Usa una estructura de datos para abrir un directorio e ir leyendo los archivos que contiene.
   Cuando encuentra un archivo cuyo nombre coincide con el archivo pasado por el usuario, devuelve el path absoluto del archivo.
   Si no encuentra el archivo en ese directorio, devuelve NULL.
*/
const char* searchFile(char* absolutePath,char* file) {

	DIR* dirToSearch;
	struct dirent *dirStruct;
	dirToSearch = opendir(absolutePath);
	if (dirToSearch)
	{
		while ((dirStruct = readdir(dirToSearch)) != NULL)
		{
			if(dirStruct->d_type == DT_REG){
					if(!(strcmp(file,dirStruct->d_name))){
						absolutePath[strlen(absolutePath)+1] = 0;
						absolutePath[strlen(absolutePath)]   = '/';
						strcat(absolutePath,dirStruct->d_name);
						return absolutePath;
					}
			}
		}
	closedir(dirToSearch);
	}
	return NULL;
}
