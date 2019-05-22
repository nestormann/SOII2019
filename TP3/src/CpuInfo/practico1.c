#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "practico1.h"
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <sys/sysinfo.h>

void upTime() {
	struct sysinfo s_info;
	int error= sysinfo(&s_info);
	const double megabyte= 1024*1024;
	if(error!=0){
		printf("error=%d\n",error);
	}
	//printf("Content-Type: text/plain;charset=us-ascii\n\n");
        printf("\n Uptime: %ld segundos\n",s_info.uptime);
	printf("\n Memoria total: %f MB\n",s_info.totalram/megabyte);
	printf("\n Memoria libre: %f MB\n",s_info.freeram/megabyte);
}

void printKernelInfo() {
	puts("\n La version del kernel es: ");
	system("uname -rms");
}

void getTime() {
	printf("Content-Type: text/plain;charset=us-ascii\n\n");
	time_t myTime;
	myTime = time(NULL);
	printf("\n Fecha y hora actual: \n %s \n", ctime(&myTime));
}

void printName() {
	puts("El nombre del equipo es:");
	system("cat /proc/sys/kernel/hostname");
}

void printCpuInfo() {

	FILE *cpuinfoFile;
	char buffer[50];
	cpuinfoFile = fopen ("/proc/cpuinfo", "r");

	if (cpuinfoFile==NULL) {
		printf("No cpuinfo file found!");
		fclose(cpuinfoFile);
		return;
	}

	puts("\n Informacion del CPU: ");

	while(!feof(cpuinfoFile)) {
   		char *tipoCpu;
		char *modelCpu;
   		char *cacheCpu;
   		char *coresCpu;
   		char *lineRead = fgets(buffer, 50, cpuinfoFile);

		if(lineRead==NULL) { break; }

	   	if((tipoCpu = strstr(lineRead, "vendor_id"))!= NULL)
    		printf("\n %s",tipoCpu);

		if((modelCpu = strstr(lineRead, "model name"))!= NULL)
			printf("%s \n",modelCpu);

		if((cacheCpu = strstr(lineRead, "cache size"))!= NULL)
			printf("%s",cacheCpu);

		if((coresCpu = strstr(lineRead, "cores"))!= NULL) {
			printf("%s",coresCpu);
			break;
			}
		}

	fclose (cpuinfoFile);
	return;
}
