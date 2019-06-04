#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv)
{
	/**
	 * Abre el archivo donde se encuentra el nombre del modulo
	 * cargado, lo lee y lo remueve.
	 * */
	char path[50] = "/var/www/html/nestor/uploads/filename.txt";
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL)
	{
			printf("Error while opening the file.\n");
			return 0;
	}
	char *file_name;
	char buff[256];
	fscanf(fp, "%s", buff);
	file_name = strtok(buff,".");
	char command[256];
	sprintf(command,"sudo rmmod %s",file_name);
	system(command);
	printf("Modulo %s removido correctamente\n",file_name);
	return 0;
}
