#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{	
	system("sudo insmod /var/www/html/nestor/uploads/modulo.ko");
	printf("Modulo instalado correctamente\n");
	return 0;
}
