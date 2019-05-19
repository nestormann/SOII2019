#define _POSIX_C_SOURCE 199309L

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>
#include <sys/stat.h>
#include <assert.h>
#include <time.h>
#include <omp.h>
#include "zlib.h"

#ifdef NAN
/* NAN is supported */
#endif

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
#define count_val NX/total_steps
#define MAX_NUM_THREADS 128 

int main(int argc, char *argv[])
{
    /*
	 *Determino el numero de hilos con el que se va a ejecutar
 	*/
 	int num_threads = 1;
 	if(argc>1)
 	 {
	 	if(atoi(argv[1]) != 0)
	 	 {
			int aux = atoi(argv[1]);
			if((aux > 0) && (aux < MAX_NUM_THREADS))
			 {
				num_threads = aux;
			 }
			else
			 {
				num_threads = omp_get_max_threads();
			 }
		 }
	 }
	omp_set_num_threads(num_threads);
    

    /** 
     * Obtengo el tamaño del archivo .nc
     * size of file is in member buffer.st_size
     * */
    struct stat buffer;
    // struct timespec starting, end;
    int status;
    status = stat(FILE_NAME, &buffer);
    size_t size_file=buffer.st_size;
    if(status != 0)
    {
        printf("Error obteniendo tamaño del archivo .nc\n");
		exit(EXIT_FAILURE);
    }
    
    int ncid, varid;
    float *data_in; 
    int retval;
    if ((retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid)))
            ERR(retval);
    data_in=malloc(NX * NY * sizeof(float));
    
    /* Obtenemos elvarID de la variable CMI. */
    if ((retval = nc_inq_varid(ncid, "CMI", &varid)))
        ERR(retval);

    /**
     * Leemos la matriz.
     * start indica la cantidad de filas (NY)
     * count indica la cantidad de columnas(NX)
     * Escribo el arreglo secuencialmente. 
     * */
    static size_t start[2]={0,0};
    static size_t count[2]={count_val,count_val};
    if ((retval = nc_get_vara_float(ncid, varid,start,count, data_in)))
        ERR(retval);
   
    //imprimir(NX, NY, "data_in_img.png", data_in, 3832);

    /**
     * Convolucion
     * */
    float kernel[3][3] = {{-1,-1,-1},{-1,8,-1},{-1,-1,-1}};
    float *convolution = malloc((NX-2) * (NY-2) * sizeof(float));
    
    double start_time = omp_get_wtime();
    #pragma omp parallel
    {
        
        for(int i=1; i < NX-1; i++)              
        {   
            #pragma omp for
            for(int j=1; j < NY-1; j++)
            {
                convolution[(i-1)*(NY-2)+(j-1)] =   data_in[(i-1)*NY+(j-1)]*kernel[0][0]+
                                                    data_in[(i-1)*NY+(j)]*kernel[0][1]+
                                                    data_in[(i-1)*NY+(j+1)]*kernel[0][2]+
                                                    data_in[(i)*NY+(j-1)]*kernel[1][0]+
                                                    data_in[(i)*NY+(j)]*kernel[1][1]+
                                                    data_in[(i)*NY+(j+1)]*kernel[1][2]+
                                                    data_in[(i+1)*NY+(j-1)]*kernel[2][0]+
                                                    data_in[(i+1)*NY+(j)]*kernel[2][1]+
                                                    data_in[(i+1)*NY+(j+1)]*kernel[2][2];               
            }
        }
    }
    double time = omp_get_wtime() - start_time;

    //imprimir(NX-2, NY-2, "convolution_img.png", convolution, 5743);
    
    /* Se cierra el archivo y liberan los recursos*/
    if ((retval = nc_close(ncid)))
        ERR(retval);
    
    printf("time: %f segundos en %d cores\n", time,num_threads);
	//ProfilerStop();
	/*
	 *Guardando tiempos de ejecucion
	*/
	FILE* f = fopen("tiempos.txt","a");
	if(!f){
		printf("Error abriendo archivo para escritura\n");
		return 1;
	}
	if(fprintf(f, "%d %f\n",num_threads, time) < 0){
		printf("Error fwrite\n");
		fclose(f);
		return 1;
	}
	fclose(f);  

    return 0;
}


    // /**
    //  * Cual es el valor maximoooo
    //  * 
    //  * */
    // float maximum = data_in[0];
 
    // for (int c = 1; c < NX*NY; c++)
    // {
    //     if (data_in[c] > maximum)
    //     {
    //         maximum  = data_in[c];
    //     }
    // }
    // printf("Maximum value is %f.\n", maximum);


