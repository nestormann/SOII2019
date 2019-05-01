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
#define total_steps 1 
#define count_val NX/total_steps-1

int
main()
{
    /** 
     * Obtengo el tamaño del archivo .nc
     * size of file is in member buffer.st_size
     * */

    struct stat buffer;
    int status;
    status = stat(FILE_NAME, &buffer);
    size_t size_file=buffer.st_size;
    if(status != 0)
    {
        printf("Error obteniendo tamaño del archivo .nc\n");
		exit(EXIT_FAILURE);
    }
    else
    {
        printf("Tamaño de %s: %ld bytes\n",FILE_NAME,size_file);
    }
    
    int ncid, varid;
    float *data_in = malloc(NX * NY * sizeof(float));

    int retval;

    if ((retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid)))
        ERR(retval);
    
    /* Obtenemos elvarID de la variable CMI. */
    if ((retval = nc_inq_varid(ncid, "CMI", &varid)))
        ERR(retval);

    /**
     * Leemos la matriz.
     * start indica la cantidad de filas (NY)
     * count indica la cantidad de columnas(NX)
     * Escribo el arreglo secuencialmente. 
     * */
    // static size_t start=0;
    // static size_t count=count_val;
    // for(int i=0;i<NY;i++)
    // {   
    //     start=0;
    //     printf("Primera data_in[%d][%ld] hasta data_in[%d][%ld] ",i,start,i,start+count);
    //     for(int j=0;j<total_steps-1;j++)
    //     {   
    //         //printf("/ %ld -  %ld / ",start,start+count-1);
    //         if ((retval = nc_get_vara_float(ncid, varid,&start,&count, data_in[i])))
    //             ERR(retval);
    //         start+=count+1;
    //     }
    //     printf("// Ultima data_in[%d][%ld] hasta data_in[%d][%ld] \n",i,start,i,start+count);
    // }

    static size_t start[2]={0,0};
    static size_t count[2]={count_val,count_val};
    // for(int i=0;i<total_steps;i++)
    // {   
        printf("Primera data_in[%ld][%ld] hasta data_in[%ld][%ld]\n",start[0],start[1],
                                                                  start[0]+count[0],start[1]+count[1]);
        if ((retval = nc_get_vara_float(ncid, varid,start,count, data_in)))
            ERR(retval);
    //     start[0]+=count[0]+1;
    //     start[1]+=count[1]+1;
    // }
    
    /* el desarrollo acá */
    FILE *fp;
    if((fp=fopen("data_in.txt", "w"))==NULL) 
    {
        printf("Cannot open file.\n");
    }
    for (int i = 3182; i < 3187; i++) 
    {   
        for(int j = 3182; j < 3187; j++)
        {
            fprintf(fp, "%.10g\t",data_in[j+i*NX]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    /* Se cierra el archivo y liberan los recursos*/
    if ((retval = nc_close(ncid)))
        ERR(retval);  

    return 0;
}


    // /** 
    //  * Obtengo el tamaño del archivo .nc
    //  * size of file is in member buffer.st_size
    //  * */
    // struct stat buffer;
    // int status;
    // status = stat(FILE_NAME, &buffer);
    // if(status != 0)
    // {
    //     printf("Error obteniendo tamaño del archivo .nc\n");
	// 	exit(EXIT_FAILURE);
    // }

    // /**
    //  * Open a netCDF file with the contents taken from a block of memory. 
    //  * */
    // status = NC_NOERR;
    // int ncid;
    // size_t size;
    // void* memory;
    // //...
    // size = buffer.st_size;
    // printf("size %ld\n", size);
    // memory = malloc(size);
    // //...
    // status = nc_open_mem(FILE_NAME, 0, size, memory, &ncid);
    // if (status != NC_NOERR) printf("FUCK\n");
    // else printf("SUCCEDED\n");
