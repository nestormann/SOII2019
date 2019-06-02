#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char** argv)
{	
	char* readInput = strchr(getenv("QUERY_STRING"),'=')+1;

	char *inputParse = strdup(readInput);

	char anio[10]={0};
	char dia[10]={0};

	strcat(anio,inputParse);
	strcat(dia,inputParse);

	char *year=strtok(dia,"&");
	char *day=strtok(NULL," ");

	day=strtok(day,"=");
	day=strtok(NULL," ");

	char *command="aws s3 ls --recursive noaa-goes16/ABI-L2-CMIPF/";
	char *flags="--no-sign-request | grep M3C13";

	char input[255];                //Entrada de usuario
	sprintf(input,"%s%s/%s/ %s",command,year,day,flags);

	system(input);
	return 0;

}
