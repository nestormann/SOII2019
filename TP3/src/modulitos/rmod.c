#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{	
	system("sudo rmmod /var/www/html/nestor/uploads/modulo.ko");
	printf("Modulo removido correctamente\n");
	return 0;
}
