all: solution reader_data_in
	   
solution: solution.o 
ifeq ($(CC),-g)
	gcc -g -pedantic -Wall -Werror -O3 -o solution solution.o `nc-config --cflags --libs` -lpng -ljpeg -lz
else
	gcc -pedantic -Wall -Werror -O3 -o solution solution.o `nc-config --cflags --libs` -lpng -ljpeg -lz
endif

solution.o: solution.c 
ifeq ($(CC),-g)
	gcc -g -pedantic -Wall -Werror -c solution.c `nc-config --cflags --libs` -lpng -ljpeg -lz
else
	gcc -pedantic -Wall -Werror -c solution.c `nc-config --cflags --libs` -lpng -ljpeg -lz
endif

reader_data_in: reader_data_in.o
	gcc -g -pedantic -Wall -Werror -O3 -o reader_data_in reader_data_in.o `nc-config --cflags --libs` 

reader_data_in.o: reader_data_in.c
	gcc -g -pedantic -Wall -Werror -c reader_data_in.c `nc-config --cflags --libs`

clean:
	rm -f *.o solution
	rm -f *.o reader_data_in
	rm data_in_img.png
	rm convolution_img.png

