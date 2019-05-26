all: baash.cgi
	   
baash.cgi: solution.o
	gcc -pedantic -Wall -Werror -O3 -o baash.cgi solution.o

solution.o: solution.c 
	gcc -pedantic -Wall -Werror -c solution.c

clean:
	rm -f *.o baash.cgi

