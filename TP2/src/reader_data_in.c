#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <netcdf_mem.h>
#include <sys/stat.h>
#include <assert.h>
#include "zlib.h"

/* Handle errors by printing an error message and exiting with a
 * non-zero status. */
#define ERRCODE 2
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);}

/* nombre del archivo a leer */
#define FILE_NAME "./asd.nc"

/* Lectura de una matriz de 21696 x 21696 */
#define NX 21696
#define NY 21696
/**
 * Los divisores de 21696 son: 1 , 2 , 3 , 4 , 6 , 8 , 12 , 16 , 24 , 32 , 
 * 48 , 64 , 96 , 113 , 192 , 226 , 339 , 452 , 678 , 904 , 1356 , 1808 , 
 * 2712 , 3616 , 5424 , 7232 , 10848 , 21696
 * */
#define total_steps NX 
#define count_val NY

int
main()
{    
    float *data_in = malloc(NX * NY * sizeof(float));
    FILE *fp;
    if((fp=fopen("data_in.txt", "r"))==NULL) 
    {
        printf("Cannot open file.\n");
    }
    if(fread(data_in, sizeof(float), NX*NY, fp) != NX*NY) 
    {
        printf("Cannot read file.\n");
        return -1;
    }
    fclose(fp);
    printf("hola: %f\n",data_in[0]);
    /**
     * i son las filas (NY)
     * j son las columnas (NX)
    */
    for (int i = 10500; i < 10505; i++)
    {
        for (int j = 10500; j < 10505; j++)
            printf("%f ", data_in[j]);
         printf("\n");
    }
    return 0;
}
