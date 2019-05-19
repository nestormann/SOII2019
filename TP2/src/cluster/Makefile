all: solution 
	   
solution: solution.c
	icc -xCORE_AVX2 -std=gnu99 -L/home/Estudiante57/Development/SO2/hpc/libs/netcdf/lib/ -lnetcdf -O3 -o solution solution.c -fopenmp

clean:
	rm -f *.o solution

