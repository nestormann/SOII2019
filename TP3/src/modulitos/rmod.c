#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{	
	FILE *fp;
	fp = fopen("filename.txt", "r"); // read mode
	if (fp == NULL)
	{
		perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}
	char file_name[256];
	fscanf(fp, "%s", file_name);
	char command[256];
	sprintf(command,"sudo rmmod /var/www/html/nestor/uploads/%s",file_name);
	system(command);
	printf("Modulo %s removido correctamente\n",file_name);
	return 0;
}
