all: solution
	   
solution: solution.o 
ifeq ($(CC),-g)
	gcc -g -pedantic -Wall -Werror -O3 -o solution solution.o -fopenmp `nc-config --cflags --libs` -lpng -ljpeg -lz
else
	gcc -pedantic -Wall -Werror -O3 -o solution solution.o -fopenmp `nc-config --cflags --libs` -lpng -ljpeg -lz
endif

solution.o: solution.c 
ifeq ($(CC),-g)
	gcc -g -pedantic -Wall -Werror -c solution.c -fopenmp `nc-config --cflags --libs` -lpng -ljpeg -lz
else
	gcc -pedantic -Wall -Werror -c solution.c -fopenmp `nc-config --cflags --libs` -lpng -ljpeg -lz
endif

clean:
	rm -f *.o solution
	rm data_in_img.png
	rm convolution_img.png

