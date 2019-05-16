#define _POSIX_C_SOURCE 199309L

#include <png.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>
#include <netcdf_mem.h>
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
#define MAX_NUM_THREADS 130 

/* A coloured pixel. */
typedef struct 
{
    // uint8_t red;
    // uint8_t green;
    // uint8_t blue;
    float alpha;
} pixel_t;

/* A picture. */
typedef struct 
{
    pixel_t *pixels;
    size_t width;
    size_t height;
} bitmap_t;

/* Given "bitmap", this returns the pixel of bitmap at the point
("x", "y"). */
static pixel_t * pixel_at(bitmap_t * bitmap, int x, int y)
{
    return bitmap->pixels + y + bitmap->height * x;
}

/* Write "bitmap" to a PNG file specified by "path"; returns 0 on
success, non-zero on error. */
static int save_png_to_file(bitmap_t *bitmap, const char *path)
{
    FILE * fp;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    size_t x, y;
    png_byte ** row_pointers = NULL;
    /* "status" contains the return value of this function. At first
    it is set to a value which means 'failure'. When the routine
    has finished its work, it is set to a value which means
    'success'. */
    int status = -1;
    /* The following number is set by trial and error only. I cannot
    see where it it is documented in the libpng manual.
    */
    int pixel_size = 4;
    int depth = 8;

    fp = fopen(path, "wb");
    if (!fp) {
        goto fopen_failed;
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        goto png_create_write_struct_failed;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        goto png_create_info_struct_failed;
    }

    /* Set up error handling. */

    if (setjmp(png_jmpbuf(png_ptr))) {
        goto png_failure;
    }

    /* Set image attributes. */

    png_set_IHDR(png_ptr,
        info_ptr,
        bitmap->width,
        bitmap->height,
        depth,
        PNG_COLOR_TYPE_GRAY,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT);

    /* Initialize rows of PNG. */

    row_pointers = png_malloc(png_ptr, bitmap->height * sizeof(png_byte *));
    for (y = 0; y < bitmap->height; ++y) {
        png_byte *row =
            png_malloc(png_ptr, sizeof(uint8_t) * bitmap->width * pixel_size);
        row_pointers[y] = row;
        for (x = 0; x < bitmap->width; ++x) {
            pixel_t * pixel = pixel_at(bitmap, x, y);
            // *row++ = pixel->red;
            // *row++ = pixel->green;
            // *row++ = pixel->blue;
            *row++ = pixel->alpha;
        }
    }

    /* Write the image data to "fp". */

    png_init_io(png_ptr, fp);
    png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    /* The routine has successfully written the file, so we set
    "status" to a value which indicates success. */

    status = 0;

    for (y = 0; y < bitmap->height; y++) {
        png_free(png_ptr, row_pointers[y]);
    }
    png_free(png_ptr, row_pointers);

    png_failure:
    png_create_info_struct_failed:
        png_destroy_write_struct(&png_ptr, &info_ptr);
    png_create_write_struct_failed:
        fclose(fp);
    fopen_failed:
        return status;
}

/* Given "value" and "max", the maximum value which we expect "value"
to take, this returns an integer between 0 and 255 proportional to
"value" divided by "max". */
static int pix(float value, float max)
{
    if (value < 0)
        return 0;
    return (int)(256.0 *((float)(value) / (float)max));
}

void imprimir(int width, int height, char * nombre_archivo,float* matriz_to_print, u_int16_t maximo)
{
    /**
     * imprimo la imagen aca 
     * */
    struct timespec starting, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &starting);
    bitmap_t img_bitmap;
    /* Create an image. */
    img_bitmap.width = width;
    img_bitmap.height = height;
    img_bitmap.pixels = calloc(sizeof(pixel_t), img_bitmap.width * img_bitmap.height);
    for (int y = 0; y < img_bitmap.height; y++) 
    {
        for (int x = 0; x < img_bitmap.width; x++) 
        {
            pixel_t * pixel = pixel_at(&img_bitmap, x, y);
            pixel->alpha = pix(matriz_to_print[x+y*width], maximo);
        }
    }
    /* Write the image to a file 'img_bitmap.png'. */
    save_png_to_file(&img_bitmap, nombre_archivo);
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    u_int64_t delta_us = (end.tv_sec - starting.tv_sec) * 1000000 + (end.tv_nsec - starting.tv_nsec) / 1000;
    u_int64_t total_time_s = delta_us/1000000;
    u_int64_t total_time_ms = (delta_us%1000000)/1000;
    printf("FINALIZO IMPRESION EN: %lds %ldms %ldus\n",total_time_s, total_time_ms, delta_us%1000);
}

float * convolution_function(float * data_in, float kernel[3][3],int num_threads)
{
    float *convolution = malloc((NX-2) * (NY-2) * sizeof(float));
    double start_time = omp_get_wtime();
    #pragma omp parallel
    {
        #pragma omp parallel for
        for(int i=1; i < NX-1; i++)              
        {   
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
    printf("time: %f con %d hilos\n", time,num_threads);
    return convolution;
}

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
    int status;
    status = stat(FILE_NAME, &buffer);
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
   
    imprimir(NX, NY, "data_in_img.png", data_in, 3832);

    /**
     * Convolucion
     * */
    float kernel[3][3] = {{-1,-1,-1},{-1,8,-1},{-1,-1,-1}};
    float *convolution = malloc((NX-2) * (NY-2) * sizeof(float));
    
    convolution_function(data_in,kernel,num_threads);

    imprimir(NX-2, NY-2, "convolution_img.png", convolution, 5743);
    
    /* Se cierra el archivo y liberan los recursos*/
    if ((retval = nc_close(ncid)))
        ERR(retval);
    
	//ProfilerStop();
	/*
	 *Guardando tiempos de ejecucion
	*/
	// FILE* f = fopen("tiempos.txt","a");
	// if(!f){
	// 	printf("Error abriendo archivo para escritura\n");
	// 	return 1;
	// }
	// if(fprintf(f, "%d %f\n",num_threads, time) < 0){
	// 	printf("Error fwrite\n");
	// 	fclose(f);
	// 	return 1;
	// }
	// fclose(f);  

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


