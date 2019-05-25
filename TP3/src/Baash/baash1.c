#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "baash.h"

#define STDOUT_FID 1
#define STDIN_FID 0
#define WRITE 1
#define	READ  0
#define MAX_BUFFER_SIZE 1024
#define MAX_PROCESSES 2
       extern char **environ;


int main(int argc, char* argv[]) {

	char* readInput = strchr(getenv("QUERY_STRING"),'=')+1;
	// int year = argv[1];
	// int day = argv[2];
	printf("readInput: %s",readInput);
	return 0;
}
